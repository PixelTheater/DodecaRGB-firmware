import sys
import os
import json
import math
from datetime import datetime
import argparse

# Add project root to Python path
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.insert(0, project_root)

from util.dodeca_core import (
    load_pcb_points,
    transform_led_point,
    radius,
    MAX_LED_NEIGHBORS
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
    print("format: index, x, y, z, led_label(1-based), side_number, [neighbors(0-based)]", file=file)
    print("*/", file=file)

    for i, point in enumerate(led_positions):
        # Format neighbors data if available
        neighbors_str = ""
        if 'distances' in point:
            neighbors = []
            for n in point['distances']:
                # Format each neighbor as a neighbor_data struct initializer
                # Keep neighbor references as 0-based indices
                neighbors.append(
                    f"{{.led_number = {n['led']}, .distance = {n['distance']:.3f}}}"
                )
            # Join all neighbors into an array initializer
            neighbors_str = f", {{{', '.join(neighbors)}}}"

        # Format the LED_Point constructor with all data
        # Use 1-based LED numbers for label_num
        line = (f"LED_Point({i}, "  # index is 0-based
                f"{point['x']:.3f}, {point['y']:.3f}, {point['z']:.3f}, "
                f"{point['label']}, {point['side']}"  # label is 1-based
                f"{neighbors_str})")
        
        if i < len(led_positions) - 1:
            line += ","
        print(line, file=file)

    print("};", file=file)

def export_points_json(led_positions, file=sys.stdout):
    """Export LED points data as JSON with distances"""
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
                'label': led['num'] + 1,  # Convert to 1-based LED numbers
                'side': side,
                'index': len(led_positions)  # Keep track of 0-based index
            })
    return led_positions

def calculate_distances(led_positions):
    """Calculate distances between all points and find nearest neighbors"""
    for i, p1 in enumerate(led_positions):
        distances = []
        for j, p2 in enumerate(led_positions):
            if i != j:
                # Calculate Euclidean distance
                dist = math.sqrt(
                    (p1['x'] - p2['x'])**2 + 
                    (p1['y'] - p2['y'])**2 + 
                    (p1['z'] - p2['z'])**2
                )
                # Only include points within reasonable distance
                if dist <= 100:
                    distances.append({
                        'led': j,  # Use 0-based index for internal references
                        'distance': dist
                    })
        
        # Sort by distance and take the nearest neighbors
        distances.sort(key=lambda x: x['distance'])
        led_positions[i]['distances'] = distances[:MAX_LED_NEIGHBORS]

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate LED point data for DodecaRGB')
    parser.add_argument('-o', '--output', help='Output file (default: stdout)')
    parser.add_argument('-f', '--format', choices=['cpp', 'json'], default='cpp',
                      help='Output format (default: cpp)')
    args = parser.parse_args()

    # Load and transform points
    pcb_points = load_pcb_points('PickAndPlace_PCB_DodecaRGB_v2_2024-11-22.csv')
    led_positions = generate_points(pcb_points)
    
    # Calculate distances
    calculate_distances(led_positions)
    
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
