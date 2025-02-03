import re
import os

def update_version_in_cpp(version_file: str, cpp_file: str) -> None:
    """Update VERSION define in main.cpp with version from VERSION file"""
    
    # Read version
    with open(version_file, 'r') as f:
        version = f.read().strip()
    
    # Read cpp file
    with open(cpp_file, 'r') as f:
        content = f.read()
    
    # Replace VERSION define
    pattern = r'#define VERSION "([^"]*)"'
    new_content = re.sub(pattern, f'#define VERSION "{version}"', content)
    
    # Write back if changed
    if new_content != content:
        print(f"Updating version in {cpp_file} to {version}")
        with open(cpp_file, 'w') as f:
            f.write(new_content)
    else:
        print(f"Version already up to date in {cpp_file}")

def main():
    # Get paths relative to script location
    script_dir = os.path.dirname(os.path.abspath(__file__))
    root_dir = os.path.dirname(os.path.dirname(script_dir))
    version_file = os.path.join(root_dir, 'VERSION')
    cpp_file = os.path.join(root_dir, 'src', 'main.cpp')
    
    update_version_in_cpp(version_file, cpp_file)

if __name__ == "__main__":
    main() 