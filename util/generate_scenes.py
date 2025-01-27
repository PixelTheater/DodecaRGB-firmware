import os
import yaml
import argparse
from datetime import datetime

def generate_parameter_struct(param_name, param_data):
    """Generate C++ parameter struct definition"""
    param_type = param_data['type']
    range_min, range_max = param_data['range']
    default = param_data.get('default', range_min)
    
    return f"""    Parameter<{param_type}> {param_name} {{
        "{param_name}",
        {range_min}, {range_max},
        {default}
    }}"""

def generate_scene_header(scene_name, yaml_data):
    """Generate C++ header file for scene"""
    class_name = ''.join(word.title() for word in scene_name.split('_'))
    parameters = yaml_data.get('parameters', {})
    
    param_structs = []
    for name, data in parameters.items():
        param_structs.append(generate_parameter_struct(name, data))
    
    return f"""// Auto-generated from {scene_name}.yaml
// Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
#pragma once
#include "PixelTheater/parameter.h"

namespace PixelTheater {{

struct {class_name}Parameters {{
{os.linesep.join(param_structs)}
}};

}} // namespace PixelTheater
"""

def process_scenes(scenes_dir, output_dir):
    """Process all YAML files in scenes directory"""
    print(f"Generating scene parameters from {scenes_dir}...")
    os.makedirs(output_dir, exist_ok=True)
    
    scenes_generated = 0
    for filename in os.listdir(scenes_dir):
        if not filename.endswith('.yaml'):
            continue
            
        scene_name = os.path.splitext(filename)[0]
        yaml_path = os.path.join(scenes_dir, filename)
        
        with open(yaml_path, 'r') as f:
            yaml_data = yaml.safe_load(f)
            
        header_content = generate_scene_header(scene_name, yaml_data)
        header_path = os.path.join(output_dir, f"{scene_name}_params.h")
        
        with open(header_path, 'w') as f:
            f.write(header_content)
            
        scenes_generated += 1
        print(f"  Created {os.path.basename(header_path)}")

    print(f"Generated {scenes_generated} scene parameter files")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--scenes', default='scenes',
                      help='Directory containing scene YAML files')
    parser.add_argument('--output', default='lib/PixelTheater/include/PixelTheater/generated',
                      help='Output directory for generated headers')
    args = parser.parse_args()
    
    process_scenes(args.scenes, args.output) 