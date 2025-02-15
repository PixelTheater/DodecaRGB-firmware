import sys
import os
import numpy as np
from datetime import datetime
import json
import math

# Add project root to Python path
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.insert(0, project_root)

from util.models import Dodecahedron

def create_model_stl(model, filename: str, scale: float = 1.0, units: str = 'mm'):
    """
    Generate STL file for any geometric model with metadata.
    
    Args:
        model: GeometricModel instance
        filename: Output STL filename
        scale: Scale factor relative to real-world size (1.0 = actual size)
        units: Output units ('mm', 'cm', 'm', 'inch')
    """
    unit_scales = {
        'mm': 1.0,
        'cm': 0.1,
        'm': 0.001,
        'inch': 1/25.4
    }
    
    if units not in unit_scales:
        raise ValueError(f"Unknown units: {units}")
    
    # Debug scaling factors
    print(f"Model scale: {model.scale}")
    print(f"User scale: {scale}")
    print(f"Real world (mm): {model.config.real_world_scale_mm}")
    print(f"Unit conversion: {unit_scales[units]}")
    
    # Convert from unit sphere to real-world scale
    real_world_scale = model.config.real_world_scale_mm * unit_scales[units]
    final_scale = real_world_scale * scale * model.scale
    
    print(f"Final scale: {final_scale}")
    
    def calculate_face_normal(vertices):
        """Calculate normal vector for a face using numpy"""
        v1 = np.array(vertices[1]) - np.array(vertices[0])
        v2 = np.array(vertices[2]) - np.array(vertices[0])
        normal = np.cross(v1, v2)
        length = np.linalg.norm(normal)
        if length < 1e-8:  # Check for zero-length normal
            return np.array([0, 0, 1])
        return normal / length

    def triangulate_face(vertices):
        """Triangulate a face into triangles"""
        vertices = np.array(vertices)  # Convert to numpy array
        # Calculate face center
        center = np.mean(vertices, axis=0)
        
        # Create triangles in counter-clockwise order
        triangles = []
        for i in range(len(vertices)):
            j = (i + 1) % len(vertices)
            # Create triangle from two adjacent vertices and center
            triangle = np.array([
                vertices[i],
                vertices[j],
                center
            ])
            triangles.append(triangle)
        return triangles

    def encode_face_data(face_index: int) -> np.uint16:
        """
        Encode face metadata into 16-bit attribute:
        - bits 0-3: face index (0-15)
        - bits 4-7: face type (0=face, 1=edge, 2=vertex)
        - bits 8-15: reserved for future use
        """
        return np.uint16(face_index & 0xF)  # For now just store face index

    # Create companion metadata file
    metadata = {
        "model": model.config.name,
        "version": "1.0",
        "generated": datetime.now().isoformat(),
        "scale": scale,
        "units": units,
        "real_world_scale_mm": model.config.real_world_scale_mm,
        "edge_length_mm": model.config.edge_length_mm,
        "faces": model.config.num_faces,
        "face_rotations": model.config.face_rotations
    }
    
    meta_file = os.path.splitext(filename)[0] + '.json'
    with open(meta_file, 'w') as f:
        json.dump(metadata, f, indent=2)

    with open(filename, 'wb') as f:
        # Write STL header with metadata reference
        header = f"DodecaRGB-v1|{model.config.name}|{os.path.basename(meta_file)}".ljust(80, '\0')
        f.write(header.encode())
        
        # Get all triangles from all faces
        all_triangles = []
        face_indices = []  # Keep track of which face each triangle belongs to
        
        faces = model.faces
        for face_index, face_vertices in enumerate(faces):
            triangles = triangulate_face(face_vertices)
            all_triangles.extend(triangles)
            face_indices.extend([face_index] * len(triangles))
            
        # Write number of triangles
        f.write(np.uint32(len(all_triangles)))
        
        # Write triangles with face metadata
        for triangle, face_index in zip(all_triangles, face_indices):
            normal = calculate_face_normal(triangle)
            f.write(normal.astype(np.float32).tobytes())
            for vertex in triangle:
                scaled_vertex = np.array(vertex) * final_scale
                f.write(scaled_vertex.astype(np.float32).tobytes())
            f.write(encode_face_data(face_index))

    print("\nVertex positions (before scaling):")
    for i, face in enumerate(faces[:1]):  # Just show first face
        print(f"\nFace {i} vertices:")
        for v in face:
            print(f"  {v} (length: {np.linalg.norm(v):.3f})")

    validate_model_size(faces, model.config.real_world_scale_mm)

def create_dodecahedron_stl(filename="dodecahedron.stl", scale=1.0):
    """Legacy wrapper for dodecahedron STL generation"""
    model = Dodecahedron(scale)
    create_model_stl(model, filename, 1.0)  # Scale already applied to model

def read_stl_metadata(filename: str) -> dict:
    """Read metadata from STL and companion file."""
    with open(filename, 'rb') as f:
        header = f.read(80).decode().rstrip('\0')
        if header.startswith('DodecaRGB-v1|'):
            _, model_name, meta_file = header.split('|')
            
            # Read companion file
            meta_path = os.path.join(os.path.dirname(filename), meta_file)
            if os.path.exists(meta_path):
                with open(meta_path) as mf:
                    metadata = json.load(mf)
                    return metadata
                    
            # Fall back to basic metadata
            return {
                "model": model_name,
                "header": header
            }
    return {}

def get_face_from_triangle(filename: str, triangle_index: int) -> int:
    """Get face index from triangle attribute data."""
    with open(filename, 'rb') as f:
        f.seek(80)  # Skip header
        num_triangles = np.frombuffer(f.read(4), dtype=np.uint32)[0]
        if triangle_index >= num_triangles:
            raise ValueError(f"Triangle index {triangle_index} out of range")
            
        # Seek to triangle's attribute bytes
        f.seek(80 + 4 + (triangle_index * 50) + 48)
        attr = np.frombuffer(f.read(2), dtype=np.uint16)[0]
        return attr & 0xF  # Extract face index

def validate_model_size(faces, scale_mm):
    """Validate model measurements"""
    # First check unscaled model
    face = faces[0]
    unscaled_edge = np.linalg.norm(np.array(face[1]) - np.array(face[0]))
    vertex_length = np.linalg.norm(np.array(face[0]))
    print(f"\nUnscaled model:")
    print(f"Vertex distance from center: {vertex_length:.3f} (should be 1.0)")
    print(f"Pentagon edge length: {unscaled_edge:.3f}")
    
    # Now check scaled measurements
    scaled_edge = unscaled_edge * scale_mm
    print(f"\nScaled measurements (mm):")
    print(f"Pentagon edge length: {scaled_edge:.2f} (target: 46.00)")
    
    # Check total height
    all_vertices = np.array([v for face in faces for v in face])
    height = (np.max(all_vertices[:,2]) - np.min(all_vertices[:,2])) * scale_mm
    print(f"Total height: {height:.2f}mm")
    
    # Calculate correct scale for 46mm edges
    correct_scale = 46.0 / unscaled_edge
    print(f"\nScaling:")
    print(f"Current scale: {scale_mm:.2f}mm")
    print(f"Scale needed for 46mm edges: {correct_scale:.2f}mm")
    print(f"Scale factor needed: {correct_scale/scale_mm:.3f}x")

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description='Generate STL file from geometric model')
    parser.add_argument('-o', '--output', default='model.stl',
                        help='Output STL filename')
    parser.add_argument('-s', '--scale', type=float, default=1.0,
                        help='Scale factor for the model')
    parser.add_argument('-m', '--model', default='dodecahedron',
                        help='Model type (currently only dodecahedron)')
    args = parser.parse_args()

    # Create model based on type
    if args.model == 'dodecahedron':
        model = Dodecahedron(args.scale)
    else:
        raise ValueError(f"Unknown model type: {args.model}")

    # Generate STL
    create_model_stl(model, args.output)
    print(f"Generated STL file: {args.output}")