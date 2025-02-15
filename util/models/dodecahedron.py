import math
from util.matrix3d import Matrix3D
from util.dodeca_core import TWO_PI
from .base import GeometricModel
from .config import DodecaConfig

class Dodecahedron(GeometricModel):
    def __init__(self, scale=1.0):
        # Create config without scale (it's now in unit sphere)
        config = DodecaConfig()
        # Pass config to parent
        super().__init__(config)
        self.matrix = Matrix3D()
        # Store scale for STL generation
        self.scale = scale
    
    @property
    def name(self) -> str:
        return self.config.name
        
    @property
    def num_faces(self) -> int:
        return self.config.num_faces
        
    @property
    def leds_per_face(self) -> int:
        return self.config.leds_per_face

    def generate_base_face(self) -> list:
        """Generate pentagon vertices on unit sphere"""
        # For a regular dodecahedron inscribed in a unit sphere:
        phi = (1 + math.sqrt(5)) / 2
        
        # Calculate pentagon vertices that will lie on unit sphere
        # These are the actual coordinates for a regular dodecahedron
        # inscribed in a unit sphere
        pentagon = []
        for i in range(5):
            angle = i * TWO_PI/5
            # These coordinates are already normalized to unit sphere
            x = 2 * math.cos(angle) / (math.sqrt(3) * phi)
            y = 2 * math.sin(angle) / (math.sqrt(3) * phi)
            z = phi / math.sqrt(3)
            pentagon.append([x, y, z])
        return pentagon

    def transform_face(self, face_index: int, matrix: Matrix3D):
        """Position face using dodecahedron transforms"""
        c = self.config
        
        # Original face positioning algorithm - critical for LED placement
        if face_index == 0:  # bottom
            matrix.rotate_z(-c.zv - c.ro*2)
        elif face_index > 0 and face_index < 6:  # bottom half
            matrix.rotate_z(c.ro*face_index + c.zv - c.ro)
            matrix.rotate_x(c.xv)
        elif face_index >= 6 and face_index < 11:  # top half
            matrix.rotate_z(c.ro*face_index - c.zv + c.ro*3)
            matrix.rotate_x(math.pi - c.xv)
        else:  # top
            matrix.rotate_x(math.pi)
            matrix.rotate_z(c.zv)
        
        # No need to translate - vertices are already on unit sphere
        matrix.rotate_z(c.zv if face_index >= 6 and face_index < 11 else -c.zv)

    def generate_faces(self) -> list:
        """Generate all faces with proper transformations"""
        pentagon = self.generate_base_face()
        faces = []
        
        for side in range(self.num_faces):
            self.matrix.push_matrix()
            self.transform_face(side, self.matrix)
            face = [self.matrix.apply(v) for v in pentagon]
            faces.append(face)
            self.matrix.pop_matrix()
            
        return faces

    def generate_led_positions(self) -> list:
        """Generate LED positions for all faces"""
        # TODO: Implement LED position generation from PCB data
        return []

    def get_face_regions(self, face_index: int) -> dict:
        """Get semantic regions for a face (center, edges, etc)"""
        # TODO: Implement face region mapping
        return {} 