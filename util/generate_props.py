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
        print(f"Error: Palette must have 2-16 entries (8-64 bytes). Found {len(palette)} bytes.", file=sys.stderr)
        return False
        
    # Validate entries and indices
    last_pos = -1
    num_entries = len(palette) // 4
    for i in range(num_entries):
        entry_start_index = i * 4
        # Check we have all components (redundant due to len check, but safe)
        if entry_start_index + 3 >= len(palette):
            print(f"Error: Incomplete entry at index {entry_start_index}.", file=sys.stderr)
            return False
            
        # Get components
        pos, r, g, b = palette[entry_start_index : entry_start_index + 4]
        
        # Validate ranges
        if not all(isinstance(x, int) and 0 <= x <= 255 
                  for x in (pos, r, g, b)):
            print(f"Error: Invalid value in entry {i} (pos={pos}, r={r}, g={g}, b={b}). Values must be 0-255.", file=sys.stderr)
            return False

        # Validate index rules
        if i == 0 and pos != 0:
            print(f"Error: First index must be 0, found {pos}.", file=sys.stderr)
            return False
        if pos <= last_pos:
            print(f"Error: Indices must be strictly increasing. Found {pos} after {last_pos}.", file=sys.stderr)
            return False
        if i == num_entries - 1 and pos != 255:
            print(f"Error: Last index must be 255, found {pos}.", file=sys.stderr)
            return False
            
        last_pos = pos
            
    return True

def generate_palette_code(name, data):
    """Generate C++ struct for palette"""
    # Convert name to valid C++ identifier
    cpp_name = name.replace('-', '_').upper()
    data_array_name = f"PALETTE_{cpp_name}_DATA"
    
    # Format data array
    data_values = [str(x) for x in data['palette']]
    
    # Generate the array definition
    array_definition = f"    constexpr uint8_t {data_array_name}[] = {{ {', '.join(data_values)} }};"

    # Generate the struct definition using the array
    struct_definition = f"""
    constexpr GradientPaletteData PALETTE_{cpp_name} = {{
        {data_array_name},
        sizeof({data_array_name})
    }};"""

    return f"""
    // {data['name']}
{array_definition}
{struct_definition}"""

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
    parser.add_argument('--palettes', default='util/palettes',
                      help='Directory containing palette files')
    parser.add_argument('--output', help='Output file (default: stdout)')
    args = parser.parse_args()
    
    try:
        palette_code = process_palettes(args.palettes)
        
        # Generate full header
        output = f"""// Auto-generated palette data
#pragma once
#include <cstdint>
#include <cstddef> // For size_t

namespace PixelTheater {{

// Definition for the Gradient Palette Data structure
struct GradientPaletteData {{
    const uint8_t* data;
    size_t size;
}};

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