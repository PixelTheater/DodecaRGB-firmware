#!/usr/bin/env python3
"""
PlatformIO script for web builds using the Makefile in the web directory.

This script follows a hybrid approach:
1. Makefile + system Emscripten handle the actual compilation
2. PlatformIO manages dependencies and testing

When used as a post-build script, it calls the Makefile to build the web application.
"""

import os
import sys
import re
import subprocess
from platformio import util

# This is CRITICAL - PlatformIO scripts must start with Import("env")
Import("env")

# Get the project directory
PROJECT_DIR = os.getcwd()

# Remove the web build artifact to force PlatformIO to rebuild
web_artifact = os.path.join(PROJECT_DIR, ".pio", "build", "web", "program")
if os.path.exists(web_artifact):
    print(f"Removing web build artifact: {web_artifact}")
    os.remove(web_artifact)

# Print debug info
print("Current CLI targets", COMMAND_LINE_TARGETS)
print("Current Build targets", BUILD_TARGETS)

# Minimum required Emscripten version
MIN_EMCC_VERSION = "3.1.0"

def check_emscripten():
    """Check if Emscripten is available and meets version requirements."""
    try:
        # Try to run emcc to check if it's available
        result = subprocess.run(["emcc", "--version"], 
                       stdout=subprocess.PIPE, 
                       stderr=subprocess.PIPE, 
                       text=True, 
                       check=True)
        
        # Extract version number using regex
        version_match = re.search(r'(\d+\.\d+\.\d+)', result.stdout)
        if not version_match:
            print("WARNING: Could not determine Emscripten version.")
            return True
        
        version = version_match.group(1)
        
        # Compare with minimum version
        min_parts = [int(x) for x in MIN_EMCC_VERSION.split('.')]
        version_parts = [int(x) for x in version.split('.')]
        
        for i in range(len(min_parts)):
            if i >= len(version_parts):
                # If we get here, min_version has more components than current_version
                # This means current_version is less than min_version
                print(f"WARNING: Emscripten version {version} is older than minimum required {MIN_EMCC_VERSION}")
                print(f"Features may not work correctly. Please consider upgrading.")
                return True
            
            if version_parts[i] > min_parts[i]:
                # Current version is newer at this component
                break
            elif version_parts[i] < min_parts[i]:
                # Current version is older at this component
                print(f"WARNING: Emscripten version {version} is older than minimum required {MIN_EMCC_VERSION}")
                print(f"Features may not work correctly. Please consider upgrading.")
                return True
        
        print(f"Using Emscripten version {version} (minimum required: {MIN_EMCC_VERSION})")
        return True
    except (subprocess.SubprocessError, FileNotFoundError):
        return False

# Check if Make is available
def check_make():
    """Check if Make is available in the PATH."""
    try:
        subprocess.run(["make", "--version"], 
                      stdout=subprocess.PIPE, 
                      stderr=subprocess.PIPE, 
                      check=True)
        return True
    except (subprocess.SubprocessError, FileNotFoundError):
        return False

# Print simple instructions for installing Emscripten
def print_emscripten_install_guide():
    """Print simple instructions for installing Emscripten."""
    print("\nEmscripten Installation:")
    print("• macOS: brew install emscripten")
    print("• All platforms: https://emscripten.org/docs/getting_started/downloads.html")

def post_program_action(source, target, env):
    """Post-build action that runs the Makefile."""
    print("\n" + "="*80)
    print("WEB BUILD POST SCRIPT IS RUNNING")
    print("="*80 + "\n")
    
    # Print program path from target
    program_path = target[0].get_abspath()
    print(f"Program path: {program_path}")
    
    # Verify requirements
    if not check_make():
        print("ERROR: 'make' not found in PATH. Please install make.")
        print("  - On macOS: Install Xcode command line tools with 'xcode-select --install'")
        print("  - On Linux: Install with your package manager, e.g., 'apt install make'")
        print("  - On Windows: Install MinGW or use WSL")
        return 1
    
    if not check_emscripten():
        print("ERROR: Emscripten (emcc) not found in PATH.")
        print_emscripten_install_guide()
        return 1
    
    # Force a rebuild or not based on environment variables or command line targets
    # If "no-clean" is in the command-line targets, don't run clean
    # skip_clean = "no-clean" in COMMAND_LINE_TARGETS
    skip_clean = True
    
    # Build the web application
    try:
        print("\n" + "="*80)
        print("BUILDING WEB APPLICATION")
        print("="*80 + "\n")
        
        print(f"Project directory: {PROJECT_DIR}")
        
        # Check if we have a Makefile in the web directory
        makefile_path = os.path.join(PROJECT_DIR, "web", "Makefile")
        if not os.path.exists(makefile_path):
            print(f"ERROR: Makefile not found at {makefile_path}")
            return 1
        
        print(f"Found Makefile at {makefile_path}")
        
        # Run make clean to force a rebuild (unless skipped)
        if not skip_clean:
            print("\nCleaning previous build with 'make clean'...")
            clean_process = subprocess.run(["make", "-f", "web/Makefile", "clean"], 
                                 check=True,
                                 stdout=subprocess.PIPE, 
                                 stderr=subprocess.PIPE,
                                 text=True)
            print("Clean completed.")
        else:
            print("\nSkipping 'make clean' as requested.")
        
        # Run make
        print("\nDelegating build to system Emscripten via Makefile...")
        process = subprocess.run(["make", "-f", "web/Makefile"], 
                             check=True,
                             stdout=subprocess.PIPE, 
                             stderr=subprocess.PIPE,
                             text=True)
        
        # Print output
        print("MAKE STDOUT:")
        print(process.stdout)
        
        if process.stderr:
            print("MAKE STDERR:")
            print(process.stderr)
            
        print("\nWeb build successful!")
        return 0
    except subprocess.CalledProcessError as e:
        print("Web build failed!")
        if hasattr(e, 'stdout') and e.stdout:
            print(e.stdout)
        if hasattr(e, 'stderr') and e.stderr:
            print(e.stderr)
        return 1

def serve_web(source, target, env):
    """Start a local web server to test the application using make serve."""
    try:
        # Run make serve
        print("Starting web server using 'make serve'...")
        subprocess.run(["make", "-f", "web/Makefile", "serve"], 
                      check=True)
    except subprocess.CalledProcessError as e:
        print("Failed to start web server:", e)
        return 1
    except KeyboardInterrupt:
        print("\nWeb server stopped.")
    
    return 0

def clean_web(source, target, env):
    """Clean the web build using make clean."""
    try:
        print("Cleaning web build...")
        subprocess.run(["make", "-f", "web/Makefile", "clean"], 
                      check=True,
                      stdout=subprocess.PIPE,
                      stderr=subprocess.PIPE,
                      text=True)
        print("Web build cleaned.")
    except subprocess.CalledProcessError as e:
        print("Failed to clean web build:", e)
        return 1
    
    return 0

# Register the post-build action
env.AddPostAction("$BUILD_DIR/${PROGNAME}${PROGSUFFIX}", post_program_action)

# Add custom targets
env.AlwaysBuild(env.Alias("serve", None, serve_web))
env.AlwaysBuild(env.Alias("clean", None, clean_web))

# Add a "no-clean" target that does nothing but can be used to signal
# the script to skip the clean step before building
def no_clean(source, target, env):
    print("Using 'no-clean' mode - will not run 'make clean' before building")
    return 0

env.AlwaysBuild(env.Alias("no-clean", None, no_clean))

# If the script is run directly (not imported by PlatformIO)
if __name__ == "__main__":
    print("This script is designed to be used with PlatformIO.")
    print("Run 'pio run -e web' to build the web application.")
    sys.exit(0) 