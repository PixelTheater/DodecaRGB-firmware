import unittest
import json
import tempfile
from pathlib import Path
from util.generate_props import validate_palette, generate_palette_code

class TestPaletteGeneration(unittest.TestCase):
    def test_validate_palette(self):
        """Test palette validation"""
        # Valid palette
        valid = {
            "name": "Test",
            "palette": [
                0,   0,   0,   128,  # Dark blue at 0%
                255, 0,   0,   128   # Dark blue at 100%
            ]
        }
        self.assertTrue(validate_palette(valid))
        
        # Invalid - missing name
        invalid1 = {
            "palette": [0, 0, 0, 0]
        }
        self.assertFalse(validate_palette(invalid1))
        
        # Invalid - wrong data type
        invalid2 = {
            "name": "Test",
            "palette": "not an array"
        }
        self.assertFalse(validate_palette(invalid2))
        
        # Invalid - incomplete entry
        invalid3 = {
            "name": "Test",
            "palette": [0, 0, 0]  # Missing blue
        }
        self.assertFalse(validate_palette(invalid3))
        
        # Invalid - out of range
        invalid4 = {
            "name": "Test",
            "palette": [0, 0, 0, 256]  # Blue > 255
        }
        self.assertFalse(validate_palette(invalid4))

    def test_generate_palette_code(self):
        """Test C++ code generation"""
        data = {
            "name": "Ocean Blue",
            "palette": [
                0,   0,   0,   128,
                255, 0,   0,   128
            ]
        }
        
        code = generate_palette_code("ocean-blue", data)
        
        # Check generated code
        self.assertIn("PALETTE_OCEAN_BLUE", code)
        self.assertIn("const uint8_t data[8]", code)
        self.assertIn("0, 0, 0, 128", code)
        self.assertIn("255, 0, 0, 128", code) 