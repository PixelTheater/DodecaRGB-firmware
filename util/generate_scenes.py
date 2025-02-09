import os
import yaml
import argparse
from datetime import datetime
import sys
from util.parameters import create_parameter

def process_scene(scene_name: str, yaml_data: dict) -> str:
    """Process scene YAML into C++ header"""
    # Validate required scene name
    if 'name' not in yaml_data:
        raise ValueError(f"Scene '{scene_name}' missing required 'name' field")
    
    # Get optional description
    description = yaml_data.get('description', '')
    
    # Process parameters section
    if 'parameters' not in yaml_data:
        raise ValueError(f"Scene '{scene_name}' missing required 'parameters' section")
    
    parameters = []
    for name, param_data in yaml_data['parameters'].items():
        try:
            param = create_parameter(name, param_data)
            parameters.append(param)
        except Exception as e:
            # Add parameter context to error
            raise type(e)(f"Parameter '{name}': {str(e)}")
    
    return generate_header(scene_name, parameters, description)

def handle_error(scene_path: str, error: Exception) -> None:
    """Format error message with file context"""
    scene_name = os.path.basename(scene_path)
    
    print(f"\nError in {scene_name}:", file=sys.stderr)
    
    if isinstance(error, KeyError):
        # Handle missing key errors more gracefully
        if str(error) == "0":
            print("  Error processing select parameter - check that 'values' is either:", file=sys.stderr)
            print("    - A list: values: ['a', 'b', 'c']", file=sys.stderr)
            print("    - A mapping: values: {a: 1, b: 2}", file=sys.stderr)
        else:
            print(f"  Missing required field: '{str(error)}'", file=sys.stderr)
            
    elif isinstance(error, AttributeError):
        print(f"  {str(error)}", file=sys.stderr)
        print("  This might be a bug in the parameter type implementation", file=sys.stderr)
    
    elif isinstance(error, yaml.parser.ParserError):
        print(f"  YAML syntax error: {str(error)}", file=sys.stderr)
        
    elif isinstance(error, ValueError):
        print(f"  {str(error)}", file=sys.stderr)
        
    else:
        print(f"  Unexpected error: {str(error)}", file=sys.stderr)
    
    # Re-raise with better context
    if isinstance(error, yaml.YAMLError):
        raise yaml.YAMLError(f"YAML error in {scene_name}: {str(error)}")
    else:
        raise type(error)(f"Error in {scene_name}: {str(error)}")

def process_scenes(scenes_dir: str, output_path: str = None) -> str:
    """Process YAML file and generate parameter code
    
    Args:
        scenes_dir: Path to YAML file
        output_path: Optional output file path. If None, output to stdout
        
    Returns:
        Generated code if output_path is None, otherwise None
    """
    # Load and process YAML
    with open(scenes_dir, 'r') as f:
        yaml_data = yaml.safe_load(f)
    
    scene_name = os.path.splitext(os.path.basename(scenes_dir))[0]
    code = process_scene(scene_name, yaml_data)
    
    # Output handling
    if output_path:
        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        with open(output_path, 'w') as f:
            f.write(code)
    else:
        return code

def format_flags(flags):
    """Convert flag list to C++ enum format"""
    if not flags or flags == ['NONE']:
        return 'Flags::NONE'
    if len(flags) == 1:
        return f"Flags::{flags[0]}"
    # For multiple flags, we'd need to implement flag combination
    # For now just use the first flag as that's what the examples show
    return f"Flags::{flags[0]}"

def generate_header(scene_name: str, parameters: list, description: str = "") -> str:
    """Generate C++ header file content"""
    # Format timestamp
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    # Start with header
    lines = [
        f"// Auto-generated from {scene_name}.yaml",
        f"// {description}",
        f"// Generated on {timestamp}",
        "#pragma once",
        '#include "PixelTheater/parameter.h"',
        "",
        "namespace PixelTheater {",
        ""
    ]
    
    # Generate option arrays for select parameters
    select_params = [p for p in parameters if p.param_type == "select"]
    for param in select_params:
        array_name = f"{param.base.name}_options"
        if isinstance(param.values, list):
            # String array for simple options
            options = [f'"{opt}"' for opt in param.values]
            lines.extend([
                f"static constexpr const char* const {array_name}[] = {{",
                f"    {', '.join(options)}, nullptr",
                "};",
            ])
        elif isinstance(param.values, dict):
            # Handle value mappings
            options = [f'"{key}"' for key in param.values.keys()]
            lines.extend([
                f"static constexpr const char* const {array_name}[] = {{",
                f"    {', '.join(options)}, nullptr",
                "};",
            ])
    
    # Generate parameter array
    lines.extend([
        "",
        "// Parameter definitions - one line per param for easy diffing",
        f"constexpr ParamDef {scene_name.upper()}_PARAMS[] = {{",
    ])
    
    # Add each parameter definition
    for param in parameters:
        flags = format_flags(param.base.flags)  # Format flags properly
        if param.param_type == "switch":
            line = f'    PARAM_SWITCH("{param.base.name}", {str(param.default).lower()}, "{param.base.description}"),'
        elif param.param_type == "count":
            line = f'    PARAM_COUNT("{param.base.name}", {param.range_min}, {param.range_max}, {param.default}, {flags}, "{param.base.description}"),'
        elif param.param_type == "range":
            line = f'    PARAM_RANGE("{param.base.name}", {param.range_min}f, {param.range_max}f, {param.default}f, {flags}, "{param.base.description}"),'
        elif param.param_type == "ratio":
            line = f'    PARAM_RATIO("{param.base.name}", {param.default}f, {flags}, "{param.base.description}"),'
        elif param.param_type == "angle":
            line = f'    PARAM_ANGLE("{param.base.name}", {param.default}f, {flags}, "{param.base.description}"),'
        elif param.param_type == "select":
            line = f'    PARAM_SELECT("{param.base.name}", 0, {param.base.name}_options, "{param.base.description}"),'
        elif param.param_type == "palette":
            line = f'    PARAM_PALETTE("{param.base.name}", "{param.default}", "{param.base.description}"),'
        else:
            raise ValueError(f"Unknown parameter type: {param.param_type}")
        
        lines.append(line)
    
    # Close array and namespace
    lines.extend([
        "};",
        "",
        "} // namespace PixelTheater",
        ""
    ])
    
    return "\n".join(lines)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate parameter code from YAML')
    parser.add_argument('input', help='Input YAML file')
    parser.add_argument('-o', '--output', help='Output header file (defaults to stdout)')
    args = parser.parse_args()
    
    result = process_scenes(args.input, args.output)
    if result:
        print(result) 