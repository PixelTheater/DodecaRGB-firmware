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
    remap_to: Optional[int] = None  # Optional geometric remapping
    position: Point3D = field(default_factory=lambda: Point3D(0, 0, 0))
    leds: List['LED'] = field(default_factory=list)

    def get_geometric_id(self) -> int:
        """Get the geometric ID for this face (uses remap_to if specified, otherwise face ID)"""
        return self.remap_to if self.remap_to is not None else self.id
    
    def is_planar(self, tolerance: float = 5.0) -> bool:
        """Check if all vertices and LEDs of this face are approximately coplanar"""
        if len(self.leds) < 3:
            return True  # Less than 3 points are always coplanar
            
        # Collect all points (vertices + LED positions)
        all_points = []
        
        # We would need access to the face vertices here, but they're calculated
        # during generation. For now, we'll check if LEDs are reasonably coplanar
        led_positions = [(led.position.x, led.position.y, led.position.z) for led in self.leds]
        
        if len(led_positions) < 4:
            return True  # Less than 4 points are always coplanar
            
        # Use first 3 points to define the plane
        p1, p2, p3 = led_positions[0], led_positions[1], led_positions[2]
        
        # Calculate normal vector using cross product
        v1 = (p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2])
        v2 = (p3[0] - p1[0], p3[1] - p1[1], p3[2] - p1[2])
        
        # Cross product
        normal = (
            v1[1] * v2[2] - v1[2] * v2[1],
            v1[2] * v2[0] - v1[0] * v2[2],
            v1[0] * v2[1] - v1[1] * v2[0]
        )
        
        # Normalize
        length = (normal[0]**2 + normal[1]**2 + normal[2]**2)**0.5
        if length < 1e-6:
            return False  # Degenerate case
        normal = (normal[0]/length, normal[1]/length, normal[2]/length)
        
        # Calculate plane equation: ax + by + cz + d = 0
        a, b, c = normal
        d = -(a * p1[0] + b * p1[1] + c * p1[2])
        
        # Check if all other points are within tolerance of this plane
        for point in led_positions[3:]:
            distance = abs(a * point[0] + b * point[1] + c * point[2] + d)
            if distance > tolerance:
                return False
                
        return True

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
                rotation=face_data['rotation'],
                remap_to=face_data.get('remap_to')  # Load remap_to if present
            )
            if 'position' in face_data:
                pos = face_data['position']
                face.position = Point3D(pos['x'], pos['y'], pos['z'])
            self.faces.append(face)
        
        # Validate remapping configuration
        self._validate_remapping()

    def _validate_remapping(self):
        """Validate face remapping configuration for errors"""
        if not self.faces:
            return
            
        # Get all face IDs and geometric IDs
        all_face_ids = {face.id for face in self.faces}
        geometric_positions = {}
        
        for face in self.faces:
            geometric_id = face.get_geometric_id()
            
            # Check if geometric ID is a valid face ID
            if geometric_id not in all_face_ids:
                raise ValueError(f"Face {face.id} has remap_to={face.remap_to} which is not a valid face ID. "
                               f"Valid face IDs are: {sorted(all_face_ids)}")
            
            # Check for duplicate geometric positions
            if geometric_id in geometric_positions:
                other_face = geometric_positions[geometric_id]
                raise ValueError(f"Multiple faces mapped to same geometric position {geometric_id}: "
                               f"Face {other_face.id} and Face {face.id}")
            
            geometric_positions[geometric_id] = face
        
        # Check that all geometric positions are covered
        geometric_ids = set(geometric_positions.keys())
        if geometric_ids != all_face_ids:
            missing = all_face_ids - geometric_ids
            if missing:
                raise ValueError(f"Missing geometric positions: {sorted(missing)}. "
                               f"All face positions must be covered by the remapping.")

class DodecaModel:
    """Manages the full dodecahedron LED model"""
    def __init__(self, model_def: ModelDefinition):
        self.model_def = model_def
        self._pcb_points = None

    def _get_rotation_for_geometric_id(self, geometric_id: int) -> int:
        """Get the rotation value for a face at the given geometric position"""
        for face in self.model_def.faces:
            if face.get_geometric_id() == geometric_id:
                return face.rotation
        return 0  # Default rotation if not found

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
                # Use geometric ID for positioning, logical ID for face assignment
                # Pass rotation from YAML config instead of using hardcoded array
                world_pos = transform_led_point(led['x'], led['y'], led['num'], face.get_geometric_id(), face.rotation)
                new_led = LED(
                    index=len(self.model_def.leds),
                    position=Point3D(*world_pos),
                    label=led['num'] + 1,  # Convert to 1-based
                    face_id=face.id  # Keep logical ID for LED assignment/wiring
                )
                self.model_def.leds.append(new_led)
                face.leds.append(new_led)

        # Calculate neighbor relationships
        self._calculate_neighbors()

    def _calculate_edges_and_relationships(self) -> List[Dict]:
        """Calculate edge geometry and face relationships"""
        edges = []
        edge_tolerance = 10.0  # mm tolerance for edge matching (proper geometric tolerance)
        
        # For each face, generate its edges
        for face in self.model_def.faces:
            face_type = self.model_def.face_types[face.type]
            geometric_id = face.get_geometric_id()
            
            # Get vertices for this face - use proper geometric calculation (no PCB offsets!)
            vertices = []
            if face_type:
                # Generate base pentagon vertices in local space (geometric shape only!)
                base_vertices = []
                num_sides = face_type.num_sides
                for j in range(num_sides):
                    angle = j * (2 * math.pi / num_sides)  # No base rotation - will be handled by LED-specific transform later
                    x = radius * math.cos(angle)
                    y = radius * math.sin(angle)
                    base_vertices.append([x, y, 0])

                # Transform vertices using proper geometric transformations (same as dodeca_core.py)
                m = Matrix3D()
                m.rotate_x(math.pi)  # Initial transform
                
                # Side positioning (using geometric ID for remapping)
                if geometric_id == 0:  # bottom
                    m.rotate_z(-zv - ro*2)
                elif geometric_id > 0 and geometric_id < 6:  # bottom half
                    m.rotate_z(ro*geometric_id + zv - ro)
                    m.rotate_x(xv)
                elif geometric_id >= 6 and geometric_id < 11:  # top half
                    m.rotate_z(ro*geometric_id - zv + ro*3)
                    m.rotate_x(math.pi - xv)
                else:  # geometric_id == 11, top
                    m.rotate_x(math.pi)
                    m.rotate_z(zv)
                
                # Move face out to radius
                m.translate(0, 0, radius*1.31)
                
                # Additional hemisphere rotation
                if geometric_id >= 6 and geometric_id < 11:
                    m.rotate_z(zv)
                else:
                    m.rotate_z(-zv)
                
                # Side rotation - use rotation from YAML config
                rotation = self._get_rotation_for_geometric_id(geometric_id)
                m.rotate_z(ro * rotation)
                
                # Add LED-specific rotation to match LED positioning
            #    m.rotate_z(math.pi/10)
                
                # Transform all vertices
                for vertex in base_vertices:
                    world_pos = m.apply(vertex)
                    # Negate Y and Z to match coordinate system
                    vertices.append([world_pos[0], -world_pos[1], -world_pos[2]])

            # Create edges from consecutive vertices
            for i in range(len(vertices)):
                start_vertex = vertices[i]
                end_vertex = vertices[(i + 1) % len(vertices)]
                
                # Find connected face for this edge
                connected_face_id = -1
                
                # Check all other faces for shared edges
                for other_face in self.model_def.faces:
                    if other_face.id == face.id:
                        continue
                        
                    other_face_type = self.model_def.face_types[other_face.type]
                    other_geometric_id = other_face.get_geometric_id()
                    
                    # Get vertices for other face - geometric shape only!
                    other_vertices = []
                    if other_face_type:
                        # Generate base vertices for geometric face shape (no PCB offsets!)
                        other_base_vertices = []
                        other_num_sides = other_face_type.num_sides
                        for j in range(other_num_sides):
                            angle = j * (2 * math.pi / other_num_sides)  # No base rotation - will be handled by LED-specific transform later
                            x = radius * math.cos(angle)
                            y = radius * math.sin(angle)
                            other_base_vertices.append([x, y, 0])

                        # Transform vertices using proper geometric transformations (same as dodeca_core.py)
                        other_m = Matrix3D()
                        other_m.rotate_x(math.pi)  # Initial transform
                        
                        # Side positioning (using geometric ID for remapping)
                        if other_geometric_id == 0:  # bottom
                            other_m.rotate_z(-zv - ro*2)
                        elif other_geometric_id > 0 and other_geometric_id < 6:  # bottom half
                            other_m.rotate_z(ro*other_geometric_id + zv - ro)
                            other_m.rotate_x(xv)
                        elif other_geometric_id >= 6 and other_geometric_id < 11:  # top half
                            other_m.rotate_z(ro*other_geometric_id - zv + ro*3)
                            other_m.rotate_x(math.pi - xv)
                        else:  # other_geometric_id == 11, top
                            other_m.rotate_x(math.pi)
                            other_m.rotate_z(zv)
                        
                        # Move face out to radius
                        other_m.translate(0, 0, radius*1.31)
                        
                        # Additional hemisphere rotation
                        if other_geometric_id >= 6 and other_geometric_id < 11:
                            other_m.rotate_z(zv)
                        else:
                            other_m.rotate_z(-zv)
                        
                        # Side rotation - use rotation from YAML config
                        other_rotation = self._get_rotation_for_geometric_id(other_geometric_id)
                        other_m.rotate_z(ro * other_rotation)
                        
                        # Add LED-specific rotation to match LED positioning
                        # other_m.rotate_z(math.pi/10)
                        
                        # Transform all vertices
                        for vertex in other_base_vertices:
                            world_pos = other_m.apply(vertex)
                            # Negate Y and Z to match coordinate system
                            other_vertices.append([world_pos[0], -world_pos[1], -world_pos[2]])

                    # Check if any edge of other face matches this edge
                    for j in range(len(other_vertices)):
                        other_start = other_vertices[j]
                        other_end = other_vertices[(j + 1) % len(other_vertices)]
                        
                        # Check if edges match (either direction)
                        def distance_3d(p1, p2):
                            return math.sqrt((p1[0]-p2[0])**2 + (p1[1]-p2[1])**2 + (p1[2]-p2[2])**2)
                        
                        # Forward match: start->end matches other_start->other_end
                        if (distance_3d(start_vertex, other_start) < edge_tolerance and 
                            distance_3d(end_vertex, other_end) < edge_tolerance):
                            connected_face_id = other_face.id
                            break
                        
                        # Reverse match: start->end matches other_end->other_start
                        if (distance_3d(start_vertex, other_end) < edge_tolerance and 
                            distance_3d(end_vertex, other_start) < edge_tolerance):
                            connected_face_id = other_face.id
                            break
                    
                    if connected_face_id != -1:
                        break
                
                edges.append({
                    'face_id': face.id,
                    'edge_index': i,
                    'start': start_vertex,
                    'end': end_vertex,
                    'connected_face_id': connected_face_id
                })
        
        return edges

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

        # Hardware metadata
        print(f"\n    // Hardware metadata from YAML configuration", file=file)
        print(f"    static constexpr HardwareData HARDWARE = {{", file=file)
        hw = self.model_def.hardware
        print(f'        .led_type = "{hw["led"]["type"]}",', file=file)
        print(f'        .color_order = "{hw["led"].get("color_order", "RGB")}",', file=file)
        print(f'        .led_diameter_mm = {hw["led"].get("diameter_mm", 2.0)}f,', file=file)
        print(f'        .led_spacing_mm = {hw["led"].get("spacing_mm", 5.0)}f,', file=file)
        print(f'        .max_current_per_led_ma = {hw["power"].get("max_current_per_led_ma", 20)},', file=file)
        print(f'        .avg_current_per_led_ma = {hw["power"].get("avg_current_per_led_ma", 10)}', file=file)
        print(f"    }};", file=file)

        # LED Groups
        print(f"\n    // LED Groups defined in YAML", file=file)
        led_groups = []
        for face_type_id, (name, ft) in enumerate(self.model_def.face_types.items()):
            for group_name, group in ft.groups.items():
                led_groups.append({
                    'name': group_name,
                    'face_type_id': face_type_id,
                    'led_count': len(group.led_indices),
                    'led_indices': group.led_indices
                })
        
        if led_groups:
            print(f"    static constexpr std::array<LedGroupData, {len(led_groups)}> LED_GROUPS{{{{", file=file)
            for i, group in enumerate(led_groups):
                print(f"        {{", file=file)
                print(f'            .name = "{group["name"]}",', file=file)
                print(f'            .face_type_id = {group["face_type_id"]},', file=file)
                print(f'            .led_count = {group["led_count"]},', file=file)
                indices_str = ', '.join([str(idx) for idx in group["led_indices"]])
                # Pad with zeros up to 32 elements
                remaining = 32 - len(group["led_indices"])
                if remaining > 0:
                    indices_str += ', ' + ', '.join(['0'] * remaining)
                print(f'            .led_indices = {{{indices_str}}}', file=file)
                print(f"        }}{'' if i == len(led_groups) - 1 else ','}", file=file)
            print("    }};", file=file)
        else:
            print(f"    static constexpr std::array<LedGroupData, 0> LED_GROUPS{{}};", file=file)

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
                    angle = 2.0 * math.pi * j / 5.0 + math.pi/10  # Add LED-specific rotation to match positioning
                    x = math.cos(angle) * ft.edge_length_mm
                    y = math.sin(angle) * ft.edge_length_mm
                    vertices.append((x, y, 0.0))
                num_sides = 5
            elif ft.name.lower() == "triangle":
                face_type = "FaceType::Triangle"
                # Calculate triangle vertices
                vertices = []
                for j in range(3):
                    angle = 2.0 * math.pi * j / 3.0 + math.pi/10  # Add LED-specific rotation to match positioning
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
            
            # Calculate vertices based on face type - geometric shape only!
            vertices = []
            if face_type:
                # Generate base vertices for geometric face shape (no PCB offsets!)
                base_vertices = []
                num_sides = face_type.num_sides
                for j in range(num_sides):
                    angle = j * (2 * math.pi / num_sides)  # No base rotation - will be handled by LED-specific transform later
                    x = radius * math.cos(angle)
                    y = radius * math.sin(angle)
                    base_vertices.append([x, y, 0])

                # Transform vertices using same pipeline as viewer
                m = Matrix3D()
                m.rotate_x(math.pi)  # Initial transform
                
                # Use geometric ID for positioning (remap-aware)
                geometric_id = face.get_geometric_id()
                
                # Side positioning from drawPentagon()
                if geometric_id == 0:  # bottom
                    m.rotate_z(-zv - ro*2)
                elif geometric_id > 0 and geometric_id < 6:  # bottom half
                    m.rotate_z(ro*geometric_id + zv - ro)
                    m.rotate_x(xv)
                elif geometric_id >= 6 and geometric_id < 11:  # top half
                    m.rotate_z(ro*geometric_id - zv + ro*3)
                    m.rotate_x(math.pi - xv)
                else:  # geometric_id == 11, top
                    m.rotate_x(math.pi)
                    m.rotate_z(zv)
                
                # Move face out to radius
                m.translate(0, 0, radius*1.31)
                
                # Additional hemisphere rotation
                if geometric_id >= 6 and geometric_id < 11:
                    m.rotate_z(zv)
                else:
                    m.rotate_z(-zv)
                
                # Side rotation - use rotation from YAML config
                rotation = self._get_rotation_for_geometric_id(geometric_id)
                m.rotate_z(ro * rotation)
                
                # Add LED-specific rotation to match LED positioning  
                m.rotate_z(math.pi/10)
                
                # Transform all vertices
                for vertex in base_vertices:
                    world_pos = m.apply(vertex)
                    # Negate Y and Z to match coordinate system
                    vertices.append([world_pos[0], -world_pos[1], -world_pos[2]])

            # Check if we have position data
            if hasattr(face.position, 'x') and face.position.x != 0 and face.position.y != 0 and face.position.z != 0:
                print(f"        {{.id = {face.id}, .type_id = {type_id}, .rotation = {face.rotation}, "
                      f".x = {face.position.x}f, .y = {face.position.y}f, .z = {face.position.z}f, "
                      f".geometric_id = {face.get_geometric_id()},", file=file)
            else:
                print(f"        {{.id = {face.id}, .type_id = {type_id}, .rotation = {face.rotation}, "
                      f".geometric_id = {face.get_geometric_id()},", file=file)
            
            # Output vertices as simple arrays of floats
            print(f"            .vertices = {{", file=file)
            total_vertices = max(len(vertices), PixelTheater.Limits.MAX_EDGES_PER_FACE)
            for j in range(total_vertices):
                if j < len(vertices):
                    vertex = vertices[j]
                    print(f"                {{.x = {vertex[0]:.3f}f, .y = {vertex[1]:.3f}f, .z = {vertex[2]:.3f}f}}{'' if j == total_vertices - 1 else ','}", file=file)
                else:
                    # Pad with zeros
                    print(f"                {{.x = 0.0f, .y = 0.0f, .z = 0.0f}}{'' if j == total_vertices - 1 else ','}", file=file)
            print(f"            }}", file=file)
            print(f"        }}{'' if i == len(self.model_def.faces) - 1 else ','}", file=file)
        print("    }};", file=file)

        # Calculate edges and face relationships
        edges = self._calculate_edges_and_relationships()
        
        # Edges
        print(f"\n    // Edge geometry and face relationships", file=file)
        if edges:
            print(f"    static constexpr std::array<EdgeData, {len(edges)}> EDGES{{{{", file=file)
            for i, edge in enumerate(edges):
                print(f"        {{", file=file)
                print(f"            .face_id = {edge['face_id']},", file=file)
                print(f"            .edge_index = {edge['edge_index']},", file=file)
                print(f"            .start_vertex = {{.x = {edge['start'][0]:.3f}f, .y = {edge['start'][1]:.3f}f, .z = {edge['start'][2]:.3f}f}},", file=file)
                print(f"            .end_vertex = {{.x = {edge['end'][0]:.3f}f, .y = {edge['end'][1]:.3f}f, .z = {edge['end'][2]:.3f}f}},", file=file)
                # Use 255 instead of -1 for no connection (max value for uint8_t)
                connected_id = 255 if edge['connected_face_id'] == -1 else edge['connected_face_id']
                print(f"            .connected_face_id = {connected_id}", file=file)
                print(f"        }}{'' if i == len(edges) - 1 else ','}", file=file)
            print("    }};", file=file)
        else:
            print(f"    static constexpr std::array<EdgeData, 0> EDGES{{}};", file=file)

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
        
        # Calculate edges for JSON export
        edges = self._calculate_edges_and_relationships()
        
        data = {
            "model": self.model_def.model,
            "geometry": self.model_def.geometry,
            "hardware": self.model_def.hardware,
            "face_types": {name: asdict(ft) for name, ft in self.model_def.face_types.items()},
            "faces": [asdict(f) for f in self.model_def.faces],
            "points": [led.to_dict() for led in self.model_def.leds],
            "edges": edges,
            "metadata": {
                "generated_date": generation_date
            }
        }
        json.dump(data, file, indent=2)

    def print_face_summary(self) -> None:
        """Print a summary of face configuration showing logical face IDs, rotations, and LED ranges"""
        print(f"\n=== Face Configuration Summary ===", file=sys.stderr)
        print(f"Total faces: {len(self.model_def.faces)}", file=sys.stderr)
        print(f"Total LEDs: {len(self.model_def.leds)}", file=sys.stderr)
        print(f"\nFace Details:", file=sys.stderr)
        print(f"{'Face ID':<8} {'Type':<10} {'Rotation':<10} {'LED Range':<15} {'LED Count':<10} {'Geometric ID':<12}", file=sys.stderr)
        print(f"{'='*8} {'='*10} {'='*10} {'='*15} {'='*10} {'='*12}", file=sys.stderr)
        
        # Sort faces by logical face ID for consistent output
        sorted_faces = sorted(self.model_def.faces, key=lambda f: f.id)
        
        for face in sorted_faces:
            if not face.leds:
                led_range = "No LEDs"
                led_count = 0
            else:
                led_indices = [led.index for led in face.leds]
                led_indices.sort()
                if len(led_indices) == 1:
                    led_range = f"{led_indices[0]}"
                else:
                    led_range = f"{led_indices[0]}-{led_indices[-1]}"
                led_count = len(led_indices)
            
            geometric_id = face.get_geometric_id()
            remap_indicator = f"{geometric_id}" if face.remap_to is None else f"{geometric_id}*"
            
            print(f"{face.id:<8} {face.type:<10} {face.rotation:<10} {led_range:<15} {led_count:<10} {remap_indicator:<12}", file=sys.stderr)
        
        # Check for face remapping
        has_remapping = any(face.remap_to is not None for face in self.model_def.faces)
        if has_remapping:
            print(f"\n* indicates faces with geometric remapping (remap_to defined)", file=sys.stderr)
            print(f"Remapping details:", file=sys.stderr)
            for face in sorted_faces:
                if face.remap_to is not None:
                    print(f"  Face {face.id} -> positioned at geometric location {face.remap_to}", file=sys.stderr)
        
        # Show LED groups if any
        led_groups = []
        for face_type_id, (name, ft) in enumerate(self.model_def.face_types.items()):
            for group_name, group in ft.groups.items():
                led_groups.append({
                    'name': group_name,
                    'face_type': name,
                    'led_count': len(group.led_indices)
                })
        
        if led_groups:
            print(f"\nLED Groups per face type:", file=sys.stderr)
            for group in led_groups:
                print(f"  {group['face_type']}.{group['name']}: {group['led_count']} LEDs", file=sys.stderr)

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

            # Print face summary
            model.print_face_summary()
        finally:
            if output_path:
                output_file.close()

    except Exception as e:
        print(f"Error: {str(e)}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main() 