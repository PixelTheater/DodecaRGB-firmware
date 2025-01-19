import unittest
import math
import numpy as np
import os
import sys

# Add project root to Python path
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, project_root)

# Now we can import our module
from util.matrix3d import Matrix3D

class TestMatrix3DBasics(unittest.TestCase):
    """Test basic matrix operations"""
    
    def setUp(self):
        self.m = Matrix3D()

    def test_identity(self):
        """Test initial matrix is identity"""
        identity = [[1, 0, 0, 0],
                   [0, 1, 0, 0],
                   [0, 0, 1, 0],
                   [0, 0, 0, 1]]
        self.assertEqual(self.m.m, identity)

    def test_translations(self):
        """Test translation operations"""
        # Test single axis translations
        self.m.translate(1, 0, 0)
        result = self.m.apply([0,0,0])
        self.assertTrue(np.allclose(result, [1,0,0]), "X translation wrong")
        
        self.m = Matrix3D()  # Reset
        self.m.translate(0, 1, 0)
        result = self.m.apply([0,0,0])
        self.assertTrue(np.allclose(result, [0,1,0]), "Y translation wrong")
        
        self.m = Matrix3D()  # Reset
        self.m.translate(0, 0, 1)
        result = self.m.apply([0,0,0])
        self.assertTrue(np.allclose(result, [0,0,1]), "Z translation wrong")

    def test_matrix_stack(self):
        """Test push/pop matrix operations"""
        self.m.translate(1, 0, 0)
        self.m.push_matrix()
        self.m.translate(0, 1, 0)
        result1 = self.m.apply([0, 0, 0])
        self.assertEqual(result1, [1, 1, 0])
        self.m.pop_matrix()
        result2 = self.m.apply([0, 0, 0])
        self.assertEqual(result2, [1, 0, 0])

class TestMatrix3DRotations(unittest.TestCase):
    """Test rotation operations"""
    
    def setUp(self):
        self.m = Matrix3D()

    def test_rotation_x(self):
        """Test rotation around X axis"""
        self.m.rotate_x(math.pi/2)  # 90 degrees
        point = [0, 1, 0]  # Point on Y axis
        result = self.m.apply(point)
        # After 90° X rotation, Y axis point should move to Z axis
        self.assertAlmostEqual(result[0], 0)
        self.assertAlmostEqual(result[1], 0)
        self.assertAlmostEqual(result[2], 1)

    def test_rotation_y(self):
        """Test rotation around Y axis"""
        self.m.rotate_y(math.pi/2)
        point = [1, 0, 0]  # Point on X axis
        result = self.m.apply(point)
        # After 90° Y rotation, X axis point should move to -Z axis
        self.assertAlmostEqual(result[0], 0)
        self.assertAlmostEqual(result[1], 0)
        self.assertAlmostEqual(result[2], -1)

    def test_rotation_z(self):
        """Test rotation around Z axis"""
        self.m.rotate_z(math.pi/2)
        point = [1, 0, 0]  # Point on X axis
        result = self.m.apply(point)
        # After 90° Z rotation, X axis point should move to Y axis
        self.assertAlmostEqual(result[0], 0)
        self.assertAlmostEqual(result[1], 1)
        self.assertAlmostEqual(result[2], 0)

class TestMatrix3DComplex(unittest.TestCase):
    """Test complex transformation sequences"""
    
    def setUp(self):
        self.m = Matrix3D()

    def test_combined_transforms(self):
        """Test combination of transformations"""
        self.m.translate(1, 0, 0)
        self.m.rotate_z(math.pi/2)
        point = [1, 0, 0]
        result = self.m.apply(point)
        # Point should be rotated then translated
        self.assertAlmostEqual(result[0], 1)
        self.assertAlmostEqual(result[1], 1)
        self.assertAlmostEqual(result[2], 0)

    def test_hemisphere_transforms(self):
        """Test transformations in different hemispheres"""
        # Test points at same radius but different hemispheres
        bottom_point = [0, 0, 200]
        top_point = [0, 0, -200]
        
        bottom_result = self.m.apply(bottom_point)
        top_result = self.m.apply(top_point)
        
        self.assertAlmostEqual(abs(bottom_result[2]), 200)
        self.assertAlmostEqual(abs(top_result[2]), 200)

class TestMatrix3DProcessing(unittest.TestCase):
    """Test compatibility with Processing's coordinate system"""
    
    def setUp(self):
        self.m = Matrix3D()

    def test_processing_coordinate_system(self):
        """Verify coordinate system matches Processing"""
        self.m.rotate_x(math.pi)
        
        test_points = [
            {'input': [0, 1, 1], 'expected': [0, -1, -1]},
            {'input': [1, 0, -1], 'expected': [1, 0, 1]},
        ]
        
        for p in test_points:
            result = self.m.apply(p['input'])
            self.assertTrue(np.allclose(result, p['expected']), 
                          f"Expected {p['expected']}, got {result}")

    def test_processing_transforms(self):
        """Test Processing-style transformation sequence"""
        self.m.rotate_x(math.pi/4)  # 45° around X
        self.m.translate(0, 0, 10)  # Translate along rotated Z
        
        result = self.m.apply([0, 0, 0])
        expected = [0, -10/math.sqrt(2), 10/math.sqrt(2)]
        self.assertTrue(np.allclose(result, expected),
                      f"Expected {expected}, got {result}")

if __name__ == '__main__':
    unittest.main(verbosity=2) 