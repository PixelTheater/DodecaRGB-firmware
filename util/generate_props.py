import os
import sys
import json
import argparse
from pathlib import Path
from PIL import Image
import datetime # Added for timestamp

__script_version__ = "1.1.0" # Simple version tracking

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

def generate_image_code(name, image_path, max_width, max_height):
    """Generate C++ struct for image data, resizing if necessary, and return code + metadata."""
    metadata = {
        "original_name": image_path.name,
        "original_width": 0,
        "original_height": 0,
        "resized": False,
        "final_width": 0,
        "final_height": 0
    }
    try:
        img = Image.open(image_path)
        original_width, original_height = img.size
        metadata["original_width"] = original_width
        metadata["original_height"] = original_height
        
        width, height = original_width, original_height
        resized = False

        # Check if resizing is needed
        if width > max_width or height > max_height:
            print(f"Info: Resizing {name} from {width}x{height} to fit within {max_width}x{max_height}...", file=sys.stderr)
            # Maintain aspect ratio
            ratio = min(max_width / width, max_height / height)
            width = int(width * ratio)
            height = int(height * ratio)
            img = img.resize((width, height), Image.Resampling.LANCZOS)
            resized = True
            print(f"Info: Resized {name} to {width}x{height}.", file=sys.stderr)
            metadata["resized"] = True
        
        metadata["final_width"] = width
        metadata["final_height"] = height
        
        img = img.convert("RGB")
        pixel_data = list(img.tobytes())

        # Convert name to valid C++ identifier
        cpp_name = name.replace('-', '_').upper()
        data_array_name = f"TEXTURE_{cpp_name}_DATA"
        
        # Format data array
        data_values = [str(x) for x in pixel_data]
        
        # Generate the array definition with PROGMEM
        # Note: Including <avr/pgmspace.h> or equivalent is needed in the C++ code
        array_definition = f"    const uint8_t {data_array_name}[] PROGMEM = {{ {', '.join(data_values)} }};"

        # Generate the struct definition using the array
        # The pointer type in TextureData should remain const uint8_t*
        # Accessing the data will require pgm_read_byte() in C++
        struct_definition = f"""
    constexpr TextureData TEXTURE_{cpp_name} = {{
        {width}, // width
        {height}, // height
        {data_array_name}
    }};"""

        # Generate comment block with metadata
        comment = f"""// Source: {metadata['original_name']} ({metadata['original_width']}x{metadata['original_height']})
    // {'Resized to: ' + str(metadata['final_width']) + 'x' + str(metadata['final_height']) if metadata['resized'] else 'Original size used.'}"""

        # Combine comment and code
        code_block = f"""
{comment}
{array_definition}
{struct_definition}"""

        return code_block, metadata # Return code and metadata dict

    except Exception as e:
        print(f"Error processing image {image_path}: {e}", file=sys.stderr)
        return None, metadata # Still return metadata even on error if possible

def process_images(image_dir, max_width, max_height):
    """Process images, generate header content with metadata."""
    image_dir = Path(image_dir)
    image_blocks = [] # Store tuples of (code_block, metadata)
    supported_extensions = ['.png', '.bmp', '.gif']
    found_images = False
    
    # Process each supported image file in the immediate directory
    for path in image_dir.glob('*'): # Only process top-level files in the dir
        if path.is_file() and path.suffix.lower() in supported_extensions:
            name = path.stem
            image_code, metadata = generate_image_code(name, path, max_width, max_height)
            if image_code:
                image_blocks.append((image_code, metadata))
                found_images = True
    
    if not found_images:
        return None # No images found or processed

    # --- Generate Header Content ---
    generation_time = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    script_name = Path(__file__).name
    
    # Extract just the code parts for joining
    image_code_parts = [block[0] for block in image_blocks]

    # Generate overall header comment
    header_comment = f"""// Auto-generated image data for {image_dir.name}
// Generated by: {script_name} v{__script_version__}
// Generation time: {generation_time}
// Max Resolution Constraint: {max_width}x{max_height}
// Source Images Processed: {len(image_blocks)}"""

    header_content = f"""{header_comment}
#pragma once
#include <cstdint>
#include <cstddef> // For size_t
#include <avr/pgmspace.h> // Include for PROGMEM - <<< ADDED FOR C++ COMPATIBILITY CHECK >>>

namespace PixelTheater {{

// Definition for the Texture Data structure
struct TextureData {{
    const uint32_t width;
    const uint32_t height;
    const uint8_t* data; // Pixel data stored row by row, R, G, B order (PROGMEM)
}};

{os.linesep.join(image_code_parts)} // Join only the code parts

}} // namespace PixelTheater
"""
    
    return header_content

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--palettes', default='util/palettes',
                      help='Directory containing palette files')
    parser.add_argument('--output', help='Output file for palettes (default: stdout)')
    parser.add_argument('--images', help='Directory containing image files to process')
    parser.add_argument('--max-resolution', default='200x100', 
                      help='Maximum resolution WxH for images (e.g., \'200x100\')')
    args = parser.parse_args()
    
    # Parse max resolution
    max_width, max_height = 200, 100 # Default
    if args.max_resolution:
        try:
            parts = args.max_resolution.lower().split('x')
            if len(parts) == 2:
                max_width = int(parts[0])
                max_height = int(parts[1])
                if max_width <= 0 or max_height <= 0:
                    raise ValueError("Resolution dimensions must be positive.")
            else:
                raise ValueError("Format must be WxH (e.g., 200x100)")
        except ValueError as e:
            print(f"Error: Invalid --max-resolution value '{args.max_resolution}': {e}", file=sys.stderr)
            return 1

    # Process Palettes
    if args.palettes:
        try:
            palette_code = process_palettes(args.palettes)
            
            # Generate full header for palettes
            palette_output = f"""// Auto-generated palette data
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
            
            # Write palette output
            if args.output:
                with open(args.output, 'w') as f:
                    f.write(palette_output)
            else:
                # If no specific palette output file, and no image processing requested, print palettes to stdout
                if not args.images:
                    print("--- Palette Data ---")
                    print(palette_output)
                
        except Exception as e:
            print(f"Error processing palettes: {e}", file=sys.stderr)
            # Decide if we should exit or continue to image processing
            # For now, let's print error and continue if possible

    # Process Images
    if args.images:
        try:
            image_dir_path = Path(args.images)
            if not image_dir_path.is_dir():
                print(f"Error: Image directory not found: {args.images}", file=sys.stderr)
                return 1
                
            image_header_content = process_images(args.images, max_width, max_height)
            
            if image_header_content:
                # Define output path within the image directory
                output_header_path = image_dir_path / "texture_data.h"
                
                # Write image output
                with open(output_header_path, 'w') as f:
                    f.write(image_header_content)
                print(f"Successfully generated {output_header_path}")
            else:
                print(f"Warning: No supported images found or processed in {args.images}.", file=sys.stderr)
        
        except Exception as e:
            print(f"Error processing images in {args.images}: {e}", file=sys.stderr)
            return 1 # Exit with error if image processing fails


    # Handle case where only images are processed and palettes are not printed to stdout
    if args.images and (not args.palettes or args.output):
         pass # Successfully processed image data, palettes handled separately or not requested for stdout

    # Check if anything was processed if respective args were given
    if args.palettes and not args.output and not palette_code and not args.images:
         print("Warning: No palettes processed or output generated.", file=sys.stderr)


    return 0 # Return 0 if script reaches end without critical errors

if __name__ == '__main__':
    sys.exit(main()) 