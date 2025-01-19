import unittest
import math
import numpy as np
import os
import sys

# Add project root to Python path
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, project_root)

# Now we can import our modules
from util.dodeca_core import (
    TWO_PI, zv, ro, xv, radius, scale,
    side_rotation, transform_led_point,
    load_pcb_points, strip_units, stripit
)

class TestDodecaCore(unittest.TestCase):
    def test_constants_exist(self):
        """Test that core constants exist and are within reasonable ranges"""
        # TWO_PI should be approximately 6.28
        self.assertTrue(6.2 < TWO_PI < 6.3)
        
        # zv (rotation between faces) should be about 0.31 (TWO_PI/20)
        self.assertTrue(0.3 < zv < 0.32)
        
        # ro (pentagon rotation) should be about 1.26 (TWO_PI/5)
        self.assertTrue(1.25 < ro < 1.27)
        
        # xv (angle between faces) should be about 1.11
        self.assertTrue(1.1 < xv < 1.12)
        
        # radius should be positive and reasonable for model size
        self.assertTrue(100 < radius < 300)
        
        # scale should be positive and reasonable for PCB dimensions
        self.assertTrue(4.0 < scale < 6.0)
    
    def test_side_rotation_validity(self):
        """Test side rotation array has valid structure"""
        # Should have exactly 12 sides
        self.assertEqual(len(side_rotation), 12)
        
        # All rotations should be integers between 0 and 4
        for rot in side_rotation:
            self.assertTrue(isinstance(rot, int))
            self.assertTrue(0 <= rot <= 4)
        
        # Bottom and top faces (0 and 11) should have rotation 0
        self.assertEqual(side_rotation[0], 0)
        self.assertEqual(side_rotation[11], 0)

    def test_strip_units(self):
        """Test stripping units from coordinate strings"""
        self.assertEqual(strip_units("123.45mm"), 123.45)
        self.assertEqual(strip_units("-0.5mm"), -0.5)
        self.assertEqual(strip_units("0mm"), 0)
        with self.assertRaises(ValueError):
            strip_units("invalid")

    def test_stripit(self):
        """Test stripping whitespace and quotes"""
        self.assertEqual(stripit('  test  '), 'test')
        self.assertEqual(stripit('"test"'), 'test')
        self.assertEqual(stripit('  "test"  '), 'test')
        self.assertEqual(stripit(''), '')

    def test_transform_led_point(self):
        """Test LED point transformation"""
        # Test center point (0,0) transformation
        center = transform_led_point(0, 0, 0, 0)
        self.assertEqual(len(center), 3)  # Should return [x,y,z]
        
        # Test points should be within model radius
        for side in [0, 5, 11]:  # Test bottom, middle, and top
            point = transform_led_point(10, 0, 0, side)
            # Point should be within reasonable bounds
            for coord in point:
                self.assertTrue(-300 < coord < 300)
        
        # Test symmetry between top and bottom faces
        p1 = transform_led_point(10, 0, 0, 0)   # Bottom
        p2 = transform_led_point(10, 0, 0, 11)  # Top
        # Z coordinates should be roughly opposite (allowing for rotations)
        self.assertTrue(p1[2] * p2[2] < 0)  # One should be positive, one negative

    def test_load_pcb_points(self):
        """Test loading PCB points from file"""
        # Test with sample data
        points = load_pcb_points('PickAndPlace_PCB_DodecaRGB_v2_2024-11-22.csv')
        
        # Should have reasonable number of points per face
        self.assertTrue(100 < len(points) < 110)
        
        # Check point structure
        self.assertTrue(all(key in points[0] for key in ['x', 'y', 'num', 'ref']))
        
        # Check coordinate ranges - PCB coordinates are scaled by 5.15
        for point in points:
            # Allow for scaled coordinates plus offset
            self.assertTrue(-200 < point['x'] < 200)
            self.assertTrue(-200 < point['y'] < 200)
            # LED numbers should be sequential within face size
            self.assertTrue(0 <= point['num'] < len(points))
            # Reference should be LED followed by a number
            self.assertTrue(point['ref'].startswith('LED'))
        
        # Test file not found
        with self.assertRaises(FileNotFoundError):
            load_pcb_points('nonexistent.csv')

if __name__ == '__main__':
    unittest.main() 