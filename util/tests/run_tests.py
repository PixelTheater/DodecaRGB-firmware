import unittest
import sys
import os
import time
from unittest.runner import TextTestResult
from termcolor import colored

# Add project root to Python path
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, project_root)

class PrettyTestResult(TextTestResult):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.successes = []  # Track successful tests
        self.start_time = time.time()

    def startTest(self, test):
        self.test_start_time = time.time()
        test_name = test.shortDescription() or str(test)
        self.stream.write(f"\n{test_name} ... ")
        self.stream.flush()

    def addSuccess(self, test):
        duration = time.time() - self.test_start_time
        self.successes.append(test)  # Track successful tests
        self.stream.write(colored("✓ ", 'green'))
        self.stream.write(f"({duration:.3f}s)")
        self.stream.flush()

    def addError(self, test, err):
        self.stream.write(colored("✗ ERROR\n", 'red'))
        self.stream.flush()
        super().addError(test, err)

    def addFailure(self, test, err):
        self.stream.write(colored("✗ FAIL\n", 'red'))
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
        """Run the tests with pretty formatting"""
        result = super().run(test)
        
        # Print summary
        self.stream.write("\n\nTest Summary:\n")
        self.stream.write("=" * 70 + "\n")
        
        # Get actual counts
        successes = len(result.successes)  # Use tracked successes
        failures = len(result.failures)
        errors = len(result.errors)
        total = successes + failures + errors
        
        # Print counts with color
        self.stream.write(f"Tests Run: {total}\n")
        if successes:
            self.stream.write(colored(f"✓ Passed:  {successes}\n", 'green'))
        if failures:
            self.stream.write(colored(f"✗ Failed:  {failures}\n", 'red'))
        if errors:
            self.stream.write(colored(f"✗ Errors:  {errors}\n", 'red'))
        
        # Print timing
        duration = time.time() - result.start_time
        self.stream.write(f"\nTotal time: {duration:.3f}s")
            
        self.stream.write("\n" + "=" * 70 + "\n")
        return result

    def _makeResult(self):
        """Create a test result with start time"""
        result = super()._makeResult()
        result.start_time = time.time()
        return result

def run_tests():
    """Run all tests in the tests directory"""
    # Discover and run tests
    loader = unittest.TestLoader()
    start_dir = os.path.dirname(__file__)
    
    # Load all test modules
    suite = unittest.TestSuite()
    for test_module in loader.discover(start_dir, pattern='test_*.py'):
        suite.addTests(test_module)
    
    # Count total tests
    total_tests = suite.countTestCases()
    if total_tests == 0:
        print("No tests found!")
        return 1
        
    # Run tests with pretty output
    runner = PrettyTestRunner(verbosity=1)
    result = runner.run(suite)
    
    # Return 0 if tests passed, 1 if any failed
    return 0 if result.wasSuccessful() else 1

if __name__ == '__main__':
    sys.exit(run_tests()) 