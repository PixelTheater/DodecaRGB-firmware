from abc import ABC, abstractmethod
import numpy as np
from typing import List, Dict, Optional
from util.matrix3d import Matrix3D
from .config import ModelConfig

class GeometricModel(ABC):
    """Base class for all geometric models"""
    
    def __init__(self, config: ModelConfig):
        self.config = config
        self._faces: Optional[List] = None
        self._led_positions: Optional[List] = None
        
    @property
    @abstractmethod
    def name(self) -> str:
        """Model identifier"""
        pass
        
    @property
    @abstractmethod
    def num_faces(self) -> int:
        """Number of faces in the model"""
        pass
        
    @property
    @abstractmethod
    def leds_per_face(self) -> int:
        """Number of LEDs on each face"""
        pass

    @property
    def faces(self) -> List:
        """Get cached face geometry"""
        if self._faces is None:
            self._faces = self.generate_faces()
        return self._faces
        
    @property
    def led_positions(self) -> List:
        """Get cached LED positions"""
        if self._led_positions is None:
            self._led_positions = self.generate_led_positions()
        return self._led_positions

    @abstractmethod
    def generate_base_face(self) -> list:
        """Generate vertices for a single face at origin"""
        pass
        
    @abstractmethod
    def transform_face(self, face_index: int, matrix: Matrix3D):
        """Apply transformations to position a face"""
        pass
        
    @abstractmethod
    def generate_faces(self) -> List:
        """Generate model faces"""
        pass
        
    @abstractmethod
    def generate_led_positions(self) -> List:
        """Generate LED positions"""
        pass
        
    @abstractmethod
    def get_face_regions(self, face_index: int) -> Dict:
        """Get semantic regions for a face"""
        pass
        
    def get_neighbors(self, led_index: int, max_distance: float = None) -> List:
        """Get neighboring LEDs within distance"""
        pass
        
    def to_cpp_header(self) -> str:
        """Generate C++ header with point definitions"""
        pass 