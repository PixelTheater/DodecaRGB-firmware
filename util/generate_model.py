#!/usr/bin/env python3

import sys
import os
import json
import math
import yaml
from datetime import datetime
from dataclasses import dataclass, asdict, field
from typing import List, Dict, Any, Optional, Tuple, Set
import argparse

# Add project root to Python path
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.insert(0, project_root)

from util.dodeca_core import (
    load_pcb_points,
    transform_led_point,
    radius,
    MAX_LED_NEIGHBORS,
    Matrix3D,
    TWO_PI,
    zv, ro, xv,
    side_rotation
)

# Define the PixelTheater namespace constants for C++ output
class PixelTheater:
    class Limits:
        MAX_NEIGHBORS = MAX_LED_NEIGHBORS
        MAX_EDGES_PER_FACE = 5

@dataclass
class Point3D:
    """Represents a point in 3D space"""
    x: float
    y: float
    z: float

    def distance_to(self, other: 'Point3D') -> float:
        """Calculate Euclidean distance to another point"""
        return math.sqrt(
            (self.x - other.x)**2 + 
            (self.y - other.y)**2 + 
            (self.z - other.z)**2
        )

@dataclass
class Neighbor:
    """Represents a neighboring LED with its distance"""
    led_number: int  # 0-based index
    distance: float

@dataclass
class LedGroup:
    """Represents a named group of LEDs on a face"""
    name: str
    led_indices: List[int]  # Local face indices

@dataclass
class FaceType:
    """Represents a type of face (pentagon, triangle, etc)"""
    name: str
    num_leds: int
    num_sides: int
    edge_length_mm: float
    groups: Dict[str, LedGroup] = field(default_factory=dict)

@dataclass
class Face:
    """Represents a face instance in the model"""
    id: int
    type: str  # References face_type name
    rotation: int
    position: Point3D = field(default_factory=lambda: Point3D(0, 0, 0))
    leds: List['LED'] = field(default_factory=list)

@dataclass
class LED:
    """Represents a single LED in the model"""
    index: int       # 0-based index in the full array
    position: Point3D
    label: int       # 1-based LED number on the PCB
    face_id: int     # Which face (0-11)
    neighbors: List[Neighbor] = None

    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary format for export"""
        return {
            'index': self.index,
            'x': self.position.x,
            'y': self.position.y,
            'z': self.position.z,
            'label': self.label,
            'face_id': self.face_id,
            'distances': [asdict(n) for n in self.neighbors] if self.neighbors else []
        }

class ModelDefinition:
    """Represents a complete model definition"""
    def __init__(self, yaml_path: str):
        with open(yaml_path, 'r') as f:
            self.config = yaml.safe_load(f)
        
        self.model = self.config['model']
        self.geometry = self.config['geometry']
        self.hardware = self.config['hardware']
        self.face_types: Dict[str, FaceType] = {}
        self.faces: List[Face] = []
        self.leds: List[LED] = []
        
        self._load_face_types()
        self._load_faces()

    def _load_face_types(self):
        """Load face type definitions from YAML"""
        for name, data in self.config['face_types'].items():
            ft = FaceType(
                name=name,
                num_leds=data['num_leds'],
                num_sides=data['num_sides'],
                edge_length_mm=self.geometry['edge_length_mm']
            )
            # Load LED groups if defined
            for group_name, indices in data.get('groups', {}).items():
                ft.groups[group_name] = LedGroup(group_name, indices)
            self.face_types[name] = ft

    def _load_faces(self):
        """Load face instances from YAML"""
        for face_data in self.config['faces']:
            face = Face(
                id=face_data['id'],
                type=face_data['type'],
                rotation=face_data['rotation']
            )
            if 'position' in face_data:
                pos = face_data['position']
                face.position = Point3D(pos['x'], pos['y'], pos['z'])
            self.faces.append(face)

class DodecaModel:
    """Manages the full dodecahedron LED model"""
    def __init__(self, model_def: ModelDefinition):
        self.model_def = model_def
        self._pcb_points = None

    def load_pcb_data(self, filename: str = None) -> None:
        """Load LED positions from PCB pick and place file"""
        if filename is None:
            filename = self.model_def.hardware['pcb']['pick_and_place_file']
        self._pcb_points = load_pcb_points(filename)

    def generate_model(self) -> None:
        """Generate the full 3D model from PCB data"""
        if not self._pcb_points:
            raise RuntimeError("PCB data must be loaded first")

        # Generate all LED positions
        for face in self.model_def.faces:
            face_type = self.model_def.face_types[face.type]
            for led in self._pcb_points:
                world_pos = transform_led_point(led['x'], led['y'], led['num'], face.id)
                new_led = LED(
                    index=len(self.model_def.leds),
                    position=Point3D(*world_pos),
                    label=led['num'] + 1,  # Convert to 1-based
                    face_id=face.id
                )
                self.model_def.leds.append(new_led)
                face.leds.append(new_led)

        # Calculate neighbor relationships
        self._calculate_neighbors()

    def _calculate_neighbors(self, max_distance: float = 100) -> None:
        """Calculate nearest neighbors for all LEDs"""
        for led in self.model_def.leds:
            distances = []
            for other in self.model_def.leds:
                if led.index != other.index:
                    dist = led.position.distance_to(other.position)
                    if dist <= max_distance:
                        distances.append(Neighbor(other.index, dist))
            
            distances.sort(key=lambda x: x.distance)
            led.neighbors = distances[:MAX_LED_NEIGHBORS]

    def export_cpp_header(self, file=sys.stdout) -> None:
        """Export model as C++ header file"""
        # Get current date and time
        generation_date = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        # Calculate sphere radius
        max_dist_sq = 0.0
        if not self.model_def.leds:
            print("Warning: No LEDs generated, cannot calculate sphere radius.", file=sys.stderr)
            sphere_radius = 0.0
        else:
            for led in self.model_def.leds:
                dist_sq = led.position.x**2 + led.position.y**2 + led.position.z**2
                if dist_sq > max_dist_sq:
                    max_dist_sq = dist_sq
            sphere_radius = math.sqrt(max_dist_sq) if max_dist_sq > 1e-9 else 0.0 # Avoid sqrt(0) or tiny negatives

        print("#pragma once", file=file)
        print("#include \"PixelTheater/model_def.h\"", file=file)
        print("#include \"PixelTheater/model/face_type.h\"", file=file)
        print(f"\n// Generated on: {generation_date}", file=file)
        
        # Add generation command info
        cmd_args = ' '.join(sys.argv[1:])
        print(f"// Generated using: {os.path.basename(sys.argv[0])} {cmd_args}", file=file)
        if hasattr(self.model_def, 'config') and 'model' in self.model_def.config:
            print(f"// Model source: {self.model_def.model.get('source', 'Unknown')}", file=file)
            if 'author' in self.model_def.model:
                print(f"// Author: {self.model_def.model['author']}", file=file)
        
        print("\nnamespace PixelTheater {", file=file)
        print("namespace Models {", file=file)
        
        # Start model definition
        model_name = self.model_def.model['name']
        led_count = len(self.model_def.leds)
        face_count = len(self.model_def.faces)
        
        print(f"\n// {self.model_def.model['description']}", file=file)
        print(f"struct {model_name} : public ModelDefinition<{led_count}, {face_count}> {{", file=file)
        
        # Required metadata
        print("\n    // Required metadata", file=file)
        print(f'    static constexpr const char* NAME = "{self.model_def.model["name"]}";', file=file)
        print(f'    static constexpr const char* VERSION = "{self.model_def.model["version"]}";', file=file)
        print(f'    static constexpr const char* DESCRIPTION = "{self.model_def.model["description"]}";', file=file)
        print(f'    static constexpr const char* MODEL_TYPE = "{self.model_def.geometry["shape"]}";', file=file)
        print(f'    static constexpr const char* GENERATED_DATE = "{generation_date}";', file=file)
        
        # Required constants
        print(f"\n    static constexpr size_t LED_COUNT = {led_count};", file=file)
        print(f"    static constexpr size_t FACE_COUNT = {face_count};", file=file)
        print(f"    static constexpr float SPHERE_RADIUS = {sphere_radius:.3f}f;", file=file)

        # Face types with vertices
        print(f"\n    // Face type definitions with vertex geometry", file=file)
        print(f"    static constexpr std::array<FaceTypeData, {len(self.model_def.face_types)}> FACE_TYPES{{{{", file=file)
        for i, (name, ft) in enumerate(self.model_def.face_types.items()):
            face_type = f"FaceType::{ft.name.capitalize()}"
            if ft.name.lower() == "pentagon":
                face_type = "FaceType::Pentagon"
                # Calculate pentagon vertices
                vertices = []
                for j in range(5):
                    angle = 2.0 * math.pi * j / 5.0
                    x = math.cos(angle) * ft.edge_length_mm
                    y = math.sin(angle) * ft.edge_length_mm
                    vertices.append((x, y, 0.0))
                num_sides = 5
            elif ft.name.lower() == "triangle":
                face_type = "FaceType::Triangle"
                # Calculate triangle vertices
                vertices = []
                for j in range(3):
                    angle = 2.0 * math.pi * j / 3.0
                    x = math.cos(angle) * ft.edge_length_mm
                    y = math.sin(angle) * ft.edge_length_mm
                    vertices.append((x, y, 0.0))
                num_sides = 3
            else:
                vertices = []
                num_sides = 0

            print(f"        {{", file=file)
            print(f"            .id = {i},", file=file)
            print(f"            .type = {face_type},", file=file)
            print(f"            .num_leds = {ft.num_leds},", file=file)
            print(f"            .edge_length_mm = {ft.edge_length_mm}f", file=file)
            print(f"        }}{'' if i == len(self.model_def.face_types) - 1 else ','}", file=file)
        print("    }};", file=file)

        # Face instances
        print(f"\n    // Face instances with transformed vertices", file=file)
        print(f"    static constexpr std::array<FaceData, FACE_COUNT> FACES{{{{", file=file)
        for i, face in enumerate(self.model_def.faces):
            # Find the type_id for this face
            type_id = 0
            face_type = None
            for j, (name, ft) in enumerate(self.model_def.face_types.items()):
                if name == face.type:
                    type_id = j
                    face_type = ft
                    break
            
            # Calculate vertices based on face type
            vertices = []
            if face_type:
                # Generate base vertices in local space for any regular polygon
                base_vertices = []
                num_sides = face_type.num_sides
                for j in range(num_sides):
                    angle = j * (2 * math.pi / num_sides)
                    x = radius * math.cos(angle)
                    y = radius * math.sin(angle)
                    base_vertices.append([x, y, 0])

                # Transform vertices using same pipeline as viewer
                m = Matrix3D()
                # No initial X rotation - keep model upright
                
                # Side positioning from drawPentagon()
                if face.id == 0:  # bottom
                    m.rotate_z(-zv - ro*2)
                elif face.id > 0 and face.id < 6:  # bottom half
                    m.rotate_z(ro*face.id + zv - ro)
                    m.rotate_x(xv)
                elif face.id >= 6 and face.id < 11:  # top half
                    m.rotate_z(ro*face.id - zv + ro*3)
                    m.rotate_x(math.pi - xv)
                else:  # face.id == 11, top
                    m.rotate_x(math.pi)
                    m.rotate_z(zv)
                
                # Move face out to radius
                m.translate(0, 0, radius*1.31)
                
                # Additional hemisphere rotation
                if face.id >= 6 and face.id < 11:
                    m.rotate_z(zv)
                else:
                    m.rotate_z(-zv)
                
                # Side rotation
                m.rotate_z(ro * side_rotation[face.id])
                
                # Transform all vertices
                for vertex in base_vertices:
                    world_pos = m.apply(vertex)
                    # Don't negate Y and Z - camera view matrix handles coordinate system
                    vertices.append([world_pos[0], world_pos[1], world_pos[2]])

            # Check if we have position data
            if hasattr(face.position, 'x') and face.position.x != 0 and face.position.y != 0 and face.position.z != 0:
                print(f"        {{.id = {face.id}, .type_id = {type_id}, .rotation = {face.rotation}, "
                      f".x = {face.position.x}f, .y = {face.position.y}f, .z = {face.position.z}f,", file=file)
            else:
                print(f"        {{.id = {face.id}, .type_id = {type_id}, .rotation = {face.rotation},", file=file)
            
            # Output vertices as simple arrays of floats
            print(f"            .vertices = {{", file=file)
            for j, vertex in enumerate(vertices):
                print(f"                {{.x = {vertex[0]:.3f}f, .y = {vertex[1]:.3f}f, .z = {vertex[2]:.3f}f}}{'' if j == len(vertices) - 1 else ','}", file=file)
            # Pad remaining vertices with zeros if needed
            for j in range(len(vertices), PixelTheater.Limits.MAX_EDGES_PER_FACE):
                print(f"                {{.x = 0.0f, .y = 0.0f, .z = 0.0f}}{'' if j == PixelTheater.Limits.MAX_EDGES_PER_FACE - 1 else ','}", file=file)
            print(f"            }}", file=file)
            print(f"        }}{'' if i == len(self.model_def.faces) - 1 else ','}", file=file)
        print("    }};", file=file)

        # Points
        print(f"\n    // Point geometry - define all points with correct face assignments", file=file)
        print(f"    static constexpr PointData POINTS[] = {{", file=file)
        for i, led in enumerate(self.model_def.leds):
            print(f"        {{{led.index}, {led.face_id}, {led.position.x:.3f}f, {led.position.y:.3f}f, {led.position.z:.3f}f}}"
                  f"{'' if i == len(self.model_def.leds) - 1 else ','}", file=file)
        print("    };", file=file)

        # Neighbors - simple array initialization
        print(f"\n    // Define neighbor relationships", file=file)
        print(f"    static constexpr NeighborData NEIGHBORS[] = {{", file=file)
        for i, led in enumerate(self.model_def.leds):
            if not led.neighbors:
                continue
            print(f"        {{{led.index}, {{", file=file)
            neighbors = []
            for n in led.neighbors[:MAX_LED_NEIGHBORS]:
                neighbors.append(f"{{.id = {n.led_number}, .distance = {n.distance:.3f}f}}")
            print(f"            {', '.join(neighbors)}", file=file)
            print(f"        }}}}{'' if i == len(self.model_def.leds) - 1 else ','}", file=file)
        print("    };", file=file)

        # Close model definition
        print("};", file=file)
        print("\n}} // namespace PixelTheater::Models", file=file)

    def export_json(self, file=sys.stdout) -> None:
        """Export model as JSON"""
        # Get current date and time
        generation_date = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        data = {
            "model": self.model_def.model,
            "geometry": self.model_def.geometry,
            "hardware": self.model_def.hardware,
            "face_types": {name: asdict(ft) for name, ft in self.model_def.face_types.items()},
            "faces": [asdict(f) for f in self.model_def.faces],
            "points": [led.to_dict() for led in self.model_def.leds],
            "metadata": {
                "generated_date": generation_date
            }
        }
        json.dump(data, file, indent=2)

def find_model_files(model_dir: str) -> tuple[str, str]:
    """Find model.yaml and pick-and-place files in a model directory structure.
    Expected structure:
    model_dir/
        model.yaml
        pcb/
            *.pos or *.csv  (pick and place file)
    """
    # Find model.yaml
    model_yaml = os.path.join(model_dir, "model.yaml")
    if not os.path.exists(model_yaml):
        raise FileNotFoundError(f"Could not find model.yaml in {model_dir}")
    
    # Find pick and place file
    pcb_dir = os.path.join(model_dir, "pcb")
    if not os.path.exists(pcb_dir):
        raise FileNotFoundError(f"Could not find pcb directory in {model_dir}")
    
    # Look for pick and place files with supported extensions
    supported_extensions = ['.pos', '.csv']
    pick_place_files = []
    for ext in supported_extensions:
        pick_place_files.extend([f for f in os.listdir(pcb_dir) if f.endswith(ext)])
    
    if not pick_place_files:
        raise FileNotFoundError(
            f"Could not find any pick-and-place files ({', '.join(supported_extensions)}) "
            f"in {pcb_dir}"
        )
    
    if len(pick_place_files) > 1:
        print(f"Warning: Multiple pick-and-place files found in {pcb_dir}, using {pick_place_files[0]}", 
              file=sys.stderr)
    
    pick_and_place = os.path.join(pcb_dir, pick_place_files[0])
    return model_yaml, pick_and_place

def confirm_overwrite(path: str, force: bool = False) -> bool:
    """Ask user to confirm file overwrite unless force is True"""
    if not os.path.exists(path) or force:
        return True
    
    response = input(f"\nFile {path} already exists. Overwrite? [y/N] ").lower()
    return response.startswith('y')

def get_output_path(model_dir: str = None, output: str = None) -> str:
    """Determine output path based on arguments"""
    if output:
        return output
    elif model_dir:
        return os.path.join(model_dir, "model.h")
    return None  # Use stdout

def main():
    parser = argparse.ArgumentParser(description='Generate LED model data for DodecaRGB')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('-m', '--model', help='Path to model YAML definition file')
    group.add_argument('-d', '--model-dir', help='Path to model directory containing model.yaml and pcb/*.pos')
    parser.add_argument('-o', '--output', help='Output file (default: model.h in model directory)')
    parser.add_argument('-f', '--format', choices=['cpp', 'json'], default='cpp',
                      help='Output format (default: cpp)')
    parser.add_argument('-i', '--input',
                      help='Input PCB pick and place file (overrides YAML definition and model-dir)')
    parser.add_argument('-y', '--yes', action='store_true',
                      help='Automatically overwrite existing files without confirmation')

    # If no arguments provided, print help and exit
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(0)

    args = parser.parse_args()

    # Validate that either model or model-dir is provided
    if not args.model and not args.model_dir:
        parser.error("either -m/--model or -d/--model-dir is required")

    try:
        # Handle model directory mode
        if args.model_dir:
            model_yaml, pick_and_place = find_model_files(args.model_dir)
            args.model = model_yaml
            if not args.input:  # Don't override if explicitly provided
                args.input = pick_and_place
        
        # Determine output path and check for overwrite
        output_path = get_output_path(args.model_dir, args.output)
        if output_path and not confirm_overwrite(output_path, args.yes):
            print("Operation cancelled.", file=sys.stderr)
            sys.exit(0)
        
        # Load model definition
        model_def = ModelDefinition(args.model)
        
        # Create and populate model
        model = DodecaModel(model_def)
        model.load_pcb_data(args.input)  # args.input can be None, will use YAML value
        model.generate_model()

        # Select output file
        output_file = open(output_path, 'w') if output_path else sys.stdout
        
        try:
            # Export in requested format
            if args.format == 'cpp':
                model.export_cpp_header(output_file)
            else:  # json
                model.export_json(output_file)
            
            if output_path:
                print(f"\nGenerated model with {len(model_def.leds)} points -> {output_path}", file=sys.stderr)
            else:
                print(f"\nGenerated model with {len(model_def.leds)} points", file=sys.stderr)
        finally:
            if output_path:
                output_file.close()

    except Exception as e:
        print(f"Error: {str(e)}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main() 