import sys
import os

# Add project root to Python path
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.insert(0, project_root)

from datetime import datetime
import argparse
import json
from dodeca_core import (
    load_pcb_points,
    transform_led_point,
    radius
)

def export_points_cpp(led_positions, file=sys.stdout):
    """Export LED points data as C++ code"""
    print("LED_Point points[] = {", file=file)
    print("/*", file=file)
    print("This text and code below was generated from a python script", file=file)
    print("using the DodecaRGB v2 PCB pick and place file.", file=file)
    print("Jeremy Seitz, 2025 - github: https://github.com/somebox/DodecaRGB-firmware", file=file)
    print("See README.md for more info.", file=file)
    print(f"Generated on {datetime.now().strftime('%d.%m.%Y - %H:%M')}", file=file)
    print(f"radius: {radius} num_leds:{len(led_positions)}", file=file)
    print("--------------", file=file)
    print("format: index, x, y, z, led_label, side_number", file=file)
    print("*/", file=file)

    for i, point in enumerate(led_positions):
        line = f"LED_Point({i}, {point['x']:.2f}, {point['y']:.2f}, {point['z']:.2f}, {point['label']}, {point['side']})"
        if i < len(led_positions) - 1:
            line += ","
        print(line, file=file)

    print("};", file=file)

def export_points_json(led_positions, file=sys.stdout):
    """Export LED points data as JSON"""
    json.dump({
        "metadata": {
            "generated": datetime.now().isoformat(),
            "radius": radius,
            "num_leds": len(led_positions)
        },
        "points": led_positions
    }, file, indent=2)

def generate_points(pcb_points):
    """Generate all LED positions from PCB points"""
    led_positions = []
    for side in range(12):
        for led in pcb_points:
            world_pos = transform_led_point(led['x'], led['y'], led['num'], side)
            led_positions.append({
                'x': world_pos[0],
                'y': world_pos[1],
                'z': world_pos[2],
                'label': led['num'],
                'side': side
            })
    return led_positions

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate LED point data for DodecaRGB')
    parser.add_argument('-o', '--output', help='Output file (default: stdout)')
    parser.add_argument('-f', '--format', choices=['cpp', 'json'], default='cpp',
                      help='Output format (default: cpp)')
    args = parser.parse_args()

    # Load and transform points
    pcb_points = load_pcb_points('PickAndPlace_PCB_DodecaRGB_v2_2024-11-22.csv')
    led_positions = generate_points(pcb_points)
    
    # Select output file
    output_file = open(args.output, 'w') if args.output else sys.stdout
    
    try:
        # Export in requested format
        if args.format == 'cpp':
            export_points_cpp(led_positions, output_file)
        else:  # json
            export_points_json(led_positions, output_file)
        
        print(f"\n// Generated {len(led_positions)} points", file=sys.stderr)
    finally:
        if args.output:
            output_file.close()
