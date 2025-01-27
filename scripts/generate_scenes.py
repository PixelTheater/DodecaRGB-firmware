Import("env")

import os
import sys
import subprocess

def run_generator(script_name, **kwargs):
    """Run a generator script with given arguments"""
    script_path = os.path.join(env["PROJECT_DIR"], "util", script_name)
    
    # Build command line arguments
    args = [sys.executable, script_path]  # Use same Python interpreter
    for key, value in kwargs.items():
        args.extend([f"--{key}", str(value)])
    
    # Run script
    result = subprocess.run(args, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error running {script_name}:", file=sys.stderr)
        print(result.stderr, file=sys.stderr)
        env.Exit(1)
    print(result.stdout)

try:
    # Get project paths
    project_dir = env["PROJECT_DIR"]
    scenes_dir = os.path.join(project_dir, "scenes")
    output_dir = os.path.join(project_dir, "lib/PixelTheater/include/PixelTheater/generated")

    # Generate scene parameters
    run_generator("generate_scenes.py",
                 scenes=scenes_dir,
                 output=output_dir)

    # Generate test fixtures if building for native
    if env["PIOENV"] == "native":
        test_fixtures_dir = os.path.join(project_dir, "test/test_fixtures")
        run_generator("generate_test_fixtures.py",
                     scenes=scenes_dir,
                     output=test_fixtures_dir)

except Exception as e:
    print(f"Error in generate_scenes.py: {e}", file=sys.stderr)
    env.Exit(1) 