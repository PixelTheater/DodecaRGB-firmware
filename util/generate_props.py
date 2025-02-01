import os
import sys
import json
import argparse
from pathlib import Path

def validate_palette(data):
    """Validate palette data structure and format"""
    # Check basic structure
    if not isinstance(data, dict):
        return False
    if 'name' not in data or 'palette' not in data:
        return False
        
    # Check palette array
    palette = data['palette']
    if not isinstance(palette, list):
        return False
    if len(palette) < 8 or len(palette) > 64:  # 2-16 entries, 4 bytes each
        return False
        
    # Validate entries
    for i in range(0, len(palette), 4):
        # Check we have all components
        if i + 3 >= len(palette):
            return False
            
        # Get components
        pos, r, g, b = palette[i:i+4]
        
        # Validate ranges
        if not all(isinstance(x, int) and 0 <= x <= 255 
                  for x in (pos, r, g, b)):
            return False
            
    return True

def generate_palette_code(name, data):
    """Generate C++ struct for palette"""
    # Convert name to valid C++ identifier
    cpp_name = name.replace('-', '_').upper()
    
    # Format data array
    data_values = [str(x) for x in data['palette']]
    
    return f"""
    // {data['name']}
    constexpr struct {{
        const uint8_t data[{len(data['palette'])}] = {{
            {', '.join(data_values)}
        }};
    }} PALETTE_{cpp_name};
    """

def process_palettes(palette_dir):
    """Process all .pal.json files in directory"""
    palette_dir = Path(palette_dir)
    output = []
    
    # Process each .pal.json file
    for path in palette_dir.glob('**/*.pal.json'):
        with open(path) as f:
            data = json.load(f)
            
        if not validate_palette(data):
            print(f"Warning: Invalid palette in {path}", file=sys.stderr)
            continue
            
        name = path.stem.replace('.pal', '')
        output.append(generate_palette_code(name, data))
    
    return output

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--palettes', default='palettes',
                      help='Directory containing palette files')
    parser.add_argument('--output', help='Output file (default: stdout)')
    args = parser.parse_args()
    
    try:
        palette_code = process_palettes(args.palettes)
        
        # Generate full header
        output = f"""// Auto-generated palette data
#pragma once
#include <cstdint>

namespace PixelTheater {{
{os.linesep.join(palette_code)}
}} // namespace PixelTheater
"""
        
        # Write output
        if args.output:
            with open(args.output, 'w') as f:
                f.write(output)
        else:
            print(output)
            
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    
    return 0

if __name__ == '__main__':
    sys.exit(main()) 