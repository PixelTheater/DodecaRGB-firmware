import unittest
import doctest
import sys
import os
import time
import subprocess
from unittest.runner import TextTestResult
from termcolor import colored
import importlib

# Add project root to Python path
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, project_root)

# Note: YAML-based parameter generation has been removed as part of the parameter system refactoring

class PrettyTestResult(TextTestResult):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.successes = []
        self.start_time = time.time()

    def startTest(self, test):
        self.test_start_time = time.time()
        test_name = test.shortDescription() or str(test)
        self.stream.write(f"\n{test_name} ... ")
        self.stream.flush()

    def addSuccess(self, test):
        duration = time.time() - self.test_start_time
        self.successes.append(test)
        self.stream.write(colored("✓ ", 'green'))
        self.stream.write(f"({duration:.3f}s)")
        self.stream.flush()

    def addError(self, test, err):
        self.stream.write(colored("✗ ERROR\n", 'red'))
        self.stream.write(f"    {err[1]}\n")
        self.stream.flush()
        super().addError(test, err)

    def addFailure(self, test, err):
        self.stream.write(colored("✗ FAIL\n", 'red'))
        self.stream.write(f"    {err[1]}\n")
        self.stream.flush()
        super().addFailure(test, err)

    def printErrors(self):
        if self.errors or self.failures:
            self.stream.write("\n\nFailures and Errors:\n")
            super().printErrors()

    def wasSuccessful(self):
        return len(self.failures) == 0 and len(self.errors) == 0

class PrettyTestRunner(unittest.TextTestRunner):
    def __init__(self, *args, **kwargs):
        kwargs['resultclass'] = PrettyTestResult
        super().__init__(*args, **kwargs)

    def run(self, test):
        "Run the given test case or test suite."
        result = self._makeResult()
        result.failfast = self.failfast
        result.buffer = self.buffer
        result.tb_locals = self.tb_locals
        
        test(result)
        
        # Skip printing any summary
        return result

def run_tests():
    """Run all tests in the tests directory"""
    # Parse verbosity from command line
    verbose = '-v' in sys.argv
    
    start_time = time.time()
    results = {}
    
    start_dir = os.path.dirname(__file__)
    
    print("\nRunning Tests:")
    print("=" * 70)
    
    # Process each test file
    for root, dirs, files in os.walk(start_dir):
        for file in files:
            if file.startswith('test_') and file.endswith('.py'):
                module_path = os.path.join(root, file)
                module_name = os.path.splitext(file)[0]
                
                # Debug test discovery
                print(f"Found test file: {module_path}")  # Debug
                
                results[module_name] = {
                    'doctest': {'run': 0, 'failed': 0},
                    'unittest': {'run': 0, 'failed': 0}
                }
                
                # Import the module
                spec = importlib.util.spec_from_file_location(module_name, module_path)
                module = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(module)
                
                # Debug test loading
                print(f"Loading tests from: {module_name}")  # Debug
                
                # Run doctests silently first to check if any exist
                result = doctest.testmod(module, verbose=False)
                if result.attempted > 0:
                    print(f"\n{module_name} (doctests):")
                    # Only show details in verbose mode
                    result = doctest.testmod(module, verbose=verbose)
                    if not verbose and result.failed == 0:
                        print(colored(f"  ✓ {result.attempted} tests passed", 'green'))
                    results[module_name]['doctest'] = {
                        'run': result.attempted,
                        'failed': result.failed
                    }
                
                # Run unittests
                loader = unittest.TestLoader()
                suite = loader.loadTestsFromModule(module)
                test_count = suite.countTestCases()
                print(f"Found {test_count} tests in {module_name}")  # Debug
                if test_count > 0:
                    print(f"\n{module_name} (unittests):")
                    runner = PrettyTestRunner(verbosity=1, stream=sys.stdout)
                    result = runner.run(suite)
                    results[module_name]['unittest'] = {
                        'run': test_count,  # Use actual test count
                        'failed': len(result.failures) + len(result.errors)
                    }
    
    # Print summary by file
    print("\nTest Results by File:")
    print("=" * 70)
    total_run = 0
    total_failed = 0
    
    for module_name, result in sorted(results.items()):
        doctest_run = result['doctest']['run']
        doctest_failed = result['doctest']['failed']
        unittest_run = result['unittest']['run']
        unittest_failed = result['unittest']['failed']
        
        if doctest_run > 0 or unittest_run > 0:
            print(f"\n{module_name}:")
            if doctest_run > 0:
                status = "✓" if doctest_failed == 0 else "✗"
                color = 'green' if doctest_failed == 0 else 'red'
                print(colored(f"  {status} Doctests: {doctest_run} run, {doctest_failed} failed", color))
            if unittest_run > 0:
                status = "✓" if unittest_failed == 0 else "✗"
                color = 'green' if unittest_failed == 0 else 'red'
                print(colored(f"  {status} Unittests: {unittest_run} run, {unittest_failed} failed", color))
        
        total_run += doctest_run + unittest_run
        total_failed += doctest_failed + unittest_failed
    
    # Print final summary
    duration = time.time() - start_time
    print("\nFinal Summary:")
    print("=" * 70)
    print(f"Total Tests Run: {total_run}")
    if total_failed == 0:
        print(colored(f"✓ All {total_run} tests passed", 'green'))
    else:
        print(colored(f"✗ {total_failed} of {total_run} tests failed", 'red'))
    print(f"\nTotal time: {duration:.3f}s")
    print("=" * 70)
    
    return 0 if total_failed == 0 else 1

if __name__ == '__main__':
    # Note: YAML-based fixture generation has been removed as part of the parameter system refactoring
    sys.exit(run_tests()) 