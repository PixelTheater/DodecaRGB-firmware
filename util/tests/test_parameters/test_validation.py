import unittest
from util.parameters.validation import create_parameter
from util.parameters.types import (
    SemanticParameter,
    RangeParameter,
    SelectParameter,
    SwitchParameter,
    ResourceParameter
)

class TestValidation(unittest.TestCase):
    def test_create_semantic_parameter(self):
        """Test creating semantic parameter"""
        param = create_parameter("test", {
            "type": "ratio",
            "default": 0.5,
            "flags": ["CLAMP"],
            "description": "Test param"
        })
        
        self.assertEqual(param.base.flags, ["CLAMP"])
        self.assertEqual(param.param_type, "ratio")
        self.assertEqual(param.default, 0.5)

    def test_create_range_parameter(self):
        param = create_parameter("count", {
            "type": "count",
            "range": [0, 100],
            "default": 50,
            "description": "Count control"
        })
        self.assertIsInstance(param, RangeParameter)
        self.assertEqual(param.range_min, 0)
        self.assertEqual(param.range_max, 100)
        self.assertEqual(param.default, 50)

    def test_create_invalid_parameter(self):
        with self.assertRaisesRegex(ValueError, "missing required 'type' field"):
            create_parameter("invalid", {})

        with self.assertRaisesRegex(ValueError, "Unknown parameter type"):
            create_parameter("invalid", {"type": "invalid"})

    def test_invalid_parameter_type(self):
        """Test error message for invalid parameter type"""
        with self.assertRaises(ValueError) as exc_info:
            create_parameter("test", {
                "type": "invalid_type",
                "default": 0,
                "description": "Test"
            })
        
        error_msg = str(exc_info.exception)
        self.assertIn("Unknown parameter type 'invalid_type'", error_msg)
        self.assertIn("Valid types are:", error_msg)
        self.assertIn("ratio", error_msg)
        self.assertIn("switch", error_msg)

    def test_invalid_flags(self):
        """Test error message for invalid flags"""
        with self.assertRaises(ValueError) as exc_info:
            create_parameter("test", {
                "type": "range",
                "range": [0, 1],
                "default": 0.5,
                "description": "Test",
                "flags": ["INVALID_FLAG"]
            })
        
        error_msg = str(exc_info.exception)
        self.assertIn("Unknown flags", error_msg)
        self.assertIn("Valid flags are:", error_msg)
        self.assertIn("CLAMP", error_msg)
        self.assertIn("WRAP", error_msg)

    def test_parameter_type_case_insensitive(self):
        """Test that parameter types are case insensitive"""
        # Test uppercase
        param = create_parameter("test", {
            "type": "SELECT",
            "values": ["a", "b"],
            "default": "a",
            "description": "Test"
        })
        self.assertIsInstance(param, SelectParameter)
        self.assertEqual(param.param_type, "select")  # Should be normalized to lowercase

        # Test mixed case
        param = create_parameter("test", {
            "type": "RaNgE",
            "range": [0, 1],
            "default": 0.5,
            "description": "Test"
        })
        self.assertIsInstance(param, RangeParameter)
        self.assertEqual(param.param_type, "range") 

    def test_flags_format(self):
        """Test that flags are properly formatted"""
        param = create_parameter("test", {
            "type": "ratio",
            "default": 0.5,
            "flags": ["clamp"],
            "description": "Test param"
        })
        
        self.assertEqual(param.base.flags, ["CLAMP"])  # Now uppercase

    def test_no_flags(self):
        """Test that no flags becomes NONE"""
        param = create_parameter("test", {
            "type": "ratio",
            "default": 0.5,
            "description": "Test param"
        })
        
        self.assertEqual(param.base.flags, ["NONE"]) 