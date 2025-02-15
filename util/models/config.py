from dataclasses import dataclass
import math
from typing import Optional

@dataclass
class ModelConfig:
    """Configuration for geometric models"""
    name: str
    num_faces: int
    leds_per_face: int
    
    # All models are defined in unit sphere (radius=1.0)
    # Real-world scale is applied during export/generation
    real_world_scale_mm: float = 54.83  # Scale for 46mm edges
    edge_length_mm: float = 46.00  # Target edge length
    
    # Optional geometric parameters
    face_rotations: list[int] = None
    face_thickness: float = None
    max_led_neighbors: int = None

class DodecaConfig(ModelConfig):
    """Dodecahedron-specific configuration"""
    def __init__(self):
        self.TWO_PI = 2 * math.pi
        self.zv = self.TWO_PI/20  # Rotation between faces
        self.ro = self.TWO_PI/5   # Rotation for pentagon points
        self.xv = 1.1071          # angle between faces
        
        super().__init__(
            name="dodecahedron",
            num_faces=12,
            leds_per_face=104,
            real_world_scale_mm=54.83,
            edge_length_mm=46.00
        )
        # Default face rotations
        self.face_rotations = [
            0,  # side 0 (bottom)
            3, 4, 4, 4, 4,  # sides 1-5
            2, 2, 2, 2, 2,  # sides 6-10
            0   # side 11 (top)
        ] 