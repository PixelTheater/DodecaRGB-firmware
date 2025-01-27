import os
from datetime import datetime

try:
    import yaml
except ImportError:
    print("Error: Missing Python dependencies.")
    print("Install required packages with:")
    print("  pip install -r util/requirements.txt")
    exit(1)

def generate_test_fixture(scene_name, yaml_data):
    """Generate C++ test fixture for scene parameters"""
    class_name = ''.join(word.title() for word in scene_name.split('_'))
    
    return f"""// Auto-generated test fixture
#pragma once
#include "PixelTheater/generated/{scene_name}_params.h"

struct {class_name}Fixture {{
    PixelTheater::{class_name}Parameters params;
    
    {class_name}Fixture() {{
        // Initialize with YAML defaults
        // No need to set defaults - constructor handles it
    }}
}};
"""

def process_test_fixtures(scenes_dir, output_dir):
    """Generate test fixtures for all scenes"""
    print(f"Generating test fixtures from {scenes_dir}...")
    os.makedirs(output_dir, exist_ok=True)
    
    fixtures_generated = 0
    for filename in os.listdir(scenes_dir):
        if not filename.endswith('.yaml'):
            continue
            
        scene_name = os.path.splitext(filename)[0]
        yaml_path = os.path.join(scenes_dir, filename)
        
        with open(yaml_path, 'r') as f:
            yaml_data = yaml.safe_load(f)
            
        fixture_content = generate_test_fixture(scene_name, yaml_data)
        fixture_path = os.path.join(output_dir, f"{scene_name}_fixture.h")
        
        with open(fixture_path, 'w') as f:
            f.write(fixture_content)
        fixtures_generated += 1
        print(f"  Created {os.path.basename(fixture_path)}")

    print(f"Generated {fixtures_generated} test fixtures") 