import math
import numpy as np

class Matrix3D:
    """3D Matrix transformation class that matches Processing's behavior"""
    def __init__(self):
        """Initialize matrix to identity"""
        self.m = [[1.0, 0.0, 0.0, 0.0],
                 [0.0, 1.0, 0.0, 0.0],
                 [0.0, 0.0, 1.0, 0.0],
                 [0.0, 0.0, 0.0, 1.0]]
        self.stack = []  # Initialize matrix stack
    
    def apply(self, point):
        """Apply transformation to point [x,y,z]"""
        x, y, z = point
        w = 1.0  # Homogeneous coordinate
        
        # Full matrix multiplication including translation
        new_x = x*self.m[0][0] + y*self.m[0][1] + z*self.m[0][2] + w*self.m[0][3]
        new_y = x*self.m[1][0] + y*self.m[1][1] + z*self.m[1][2] + w*self.m[1][3]
        new_z = x*self.m[2][0] + y*self.m[2][1] + z*self.m[2][2] + w*self.m[2][3]
        
        return [new_x, new_y, new_z]

    def rotate_x(self, angle: float):
        """Rotate around X axis by angle (radians)"""
        c = math.cos(angle)
        s = math.sin(angle)
        rot = [[1,  0,   0, 0],
               [0,  c,  -s, 0],
               [0,  s,   c, 0],
               [0,  0,   0, 1]]
        self.m = self._multiply_matrices(self.m, rot)

    def rotate_z(self, angle: float):
        """Rotate around Z axis by angle (radians)"""
        c = math.cos(angle)
        s = math.sin(angle)
        rot = [[ c, -s, 0, 0],
               [ s,  c, 0, 0],
               [ 0,  0, 1, 0],
               [ 0,  0, 0, 1]]
        self.m = self._multiply_matrices(self.m, rot)

    def rotate_y(self, angle):
        """Rotate around Y axis by given angle in radians"""
        c = math.cos(angle)
        s = math.sin(angle)
        
        # Y rotation matrix - corrected signs for right-handed coordinate system
        r = [[ c,  0,  s, 0],
             [ 0,  1,  0, 0],
             [-s,  0,  c, 0],
             [ 0,  0,  0, 1]]
        
        self.m = self._multiply_matrices(self.m, r)

    def _multiply_matrices(self, a, b):
        """Multiply two 4x4 matrices"""
        result = [[0 for _ in range(4)] for _ in range(4)]
        for i in range(4):
            for j in range(4):
                for k in range(4):
                    result[i][j] += a[i][k] * b[k][j]
        return result

    def translate(self, x: float, y: float, z: float):
        """Translate by (x,y,z)"""
        trans = [[1, 0, 0, x],
                [0, 1, 0, y],
                [0, 0, 1, z],
                [0, 0, 0, 1]]
        self.m = self._multiply_matrices(self.m, trans)

    def push_matrix(self):
        """Save current matrix state"""
        self.stack.append([row[:] for row in self.m])

    def pop_matrix(self):
        """Restore previous matrix state"""
        if not self.stack:
            raise Exception("Matrix stack is empty")
        self.m = self.stack.pop() 