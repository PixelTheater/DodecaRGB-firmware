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
    MAX_LED_NEIGHBORS
)

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
        print("#pragma once", file=file)
        print("\n#include <cstdint>", file=file)
        print("#include \"model_types.h\"", file=file)
        print("\nnamespace Models {", file=file)
        
        # Start model definition
        print(f"\nstruct {self.model_def.model['name']} {{", file=file)
        print(f"    static constexpr uint16_t LED_COUNT = {len(self.model_def.leds)};", file=file)
        print(f"    static constexpr uint8_t FACE_COUNT = {len(self.model_def.faces)};", file=file)
        
        # Metadata
        print("\n    struct Metadata {", file=file)
        print(f'        static constexpr const char* name = "{self.model_def.model["name"]}";', file=file)
        print(f'        static constexpr const char* version = "{self.model_def.model["version"]}";', file=file)
        print(f'        static constexpr const char* description = "{self.model_def.model["description"]}";', file=file)
        print("    };", file=file)

        # Geometry
        print("\n    struct Geometry {", file=file)
        print(f'        static constexpr const char* shape = "{self.model_def.geometry["shape"]}";', file=file)
        print(f'        static constexpr float edge_length_mm = {self.model_def.geometry["edge_length_mm"]}f;', file=file)
        print(f'        static constexpr float radius_mm = {self.model_def.geometry["radius_mm"]}f;', file=file)
        print("    };", file=file)

        # Face types
        print("\n    PROGMEM static constexpr FaceTypeData face_types[] = {", file=file)
        for ft in self.model_def.face_types.values():
            print(f"        {{ \"{ft.name}\", {ft.num_sides}, {ft.num_leds}, {ft.edge_length_mm}f }},", file=file)
        print("    };", file=file)

        # Face instances
        print("\n    PROGMEM static constexpr FaceData faces[] = {", file=file)
        for face in self.model_def.faces:
            print(f"        {{ {face.id}, \"{face.type}\", {face.rotation}, "
                  f"{face.position.x}f, {face.position.y}f, {face.position.z}f }},", file=file)
        print("    };", file=file)

        # Points
        print("\n    PROGMEM static constexpr PointData points[] = {", file=file)
        for led in self.model_def.leds:
            print(f"        {{ {led.index}, {led.face_id}, {led.position.x:.3f}f, "
                  f"{led.position.y:.3f}f, {led.position.z:.3f}f }},", file=file)
        print("    };", file=file)

        # Neighbors
        print("\n    PROGMEM static constexpr NeighborData neighbors[] = {", file=file)
        for led in self.model_def.leds:
            neighbors_str = ", ".join(
                f"{{ {n.led_number}, {n.distance:.3f}f }}"
                for n in (led.neighbors or [])
            )
            print(f"        {{ {led.index}, {{ {neighbors_str} }} }},", file=file)
        print("    };", file=file)

        # Close model definition
        print("};", file=file)
        print("\n} // namespace Models", file=file)

    def export_json(self, file=sys.stdout) -> None:
        """Export model as JSON"""
        data = {
            "model": self.model_def.model,
            "geometry": self.model_def.geometry,
            "hardware": self.model_def.hardware,
            "face_types": {name: asdict(ft) for name, ft in self.model_def.face_types.items()},
            "faces": [asdict(f) for f in self.model_def.faces],
            "points": [led.to_dict() for led in self.model_def.leds]
        }
        json.dump(data, file, indent=2)

def main():
    parser = argparse.ArgumentParser(description='Generate LED model data for DodecaRGB')
    parser.add_argument('-o', '--output', help='Output file (default: stdout)')
    parser.add_argument('-f', '--format', choices=['cpp', 'json'], default='cpp',
                      help='Output format (default: cpp)')
    parser.add_argument('-m', '--model', required=True,
                      help='Path to model YAML definition file')
    parser.add_argument('-i', '--input',
                      help='Input PCB pick and place file (overrides YAML definition)')
    args = parser.parse_args()

    try:
        # Load model definition
        model_def = ModelDefinition(args.model)
        
        # Create and populate model
        model = DodecaModel(model_def)
        model.load_pcb_data(args.input)  # args.input can be None, will use YAML value
        model.generate_model()

        # Select output file
        output_file = open(args.output, 'w') if args.output else sys.stdout
        
        try:
            # Export in requested format
            if args.format == 'cpp':
                model.export_cpp_header(output_file)
            else:  # json
                model.export_json(output_file)
            
            print(f"\n// Generated model with {len(model_def.leds)} points", file=sys.stderr)
        finally:
            if args.output:
                output_file.close()

    except Exception as e:
        print(f"Error: {str(e)}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main() 