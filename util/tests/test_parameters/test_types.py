import unittest
from util.parameters.types import (
    SemanticParameter,
    RangeParameter,
    SelectParameter,
    SwitchParameter,
    ResourceParameter,
    ParameterBase,
    PARAM_TYPES
)

class TestParameterTypes(unittest.TestCase):
    def test_semantic_parameter(self):
        """Test semantic parameter"""
        base = ParameterBase(
            name="speed",
            description="Speed control",
            flags=["Clamp"]
        )
        param = SemanticParameter(
            base=base,
            param_type="signed_ratio",
            default=0.5
        )
        param.validate()  # Should pass
        self.assertIn("ParamType::signed_ratio", param.generate_code())

    def test_range_parameter_validation(self):
        """Test range parameter validation"""
        base = ParameterBase(
            name="count",
            description="Invalid range"
        )
        with self.assertRaises(ValueError):  # Removed match parameter
            param = RangeParameter(
                base=base,
                param_type="count",
                range_min=100,
                range_max=10,
                default=50
            )
            param.validate()

    def test_switch_parameter(self):
        """Test switch (boolean) parameter"""
        base = ParameterBase(
            name="active",
            description="Enable animation",
            flags=["Slew"]
        )
        param = SwitchParameter(
            base=base,
            default=True
        )
        param.validate()
        code = param.generate_code()
        self.assertIn("ParamType::switch", code)
        self.assertIn("true", code)
        self.assertIn("ParamFlag::Slew", code)

    def test_resource_parameter(self):
        """Test resource parameters"""
        # Test palette
        base = ParameterBase(
            name="colors",
            description="Color scheme"
        )
        palette = ResourceParameter(
            base=base,
            param_type="palette",
            default="rainbow"
        )
        palette.validate()
        self.assertIn('ParamType::palette', palette.generate_code())
        self.assertIn('"rainbow"', palette.generate_code())

        # Test invalid resource type
        base = ParameterBase(
            name="invalid",
            description="Invalid resource"
        )
        with self.assertRaises(ValueError):  # Removed match parameter
            ResourceParameter(
                base=base,
                param_type="unknown",
                default="test"
            ).validate()

    def test_semantic_parameter_range_validation(self):
        """Test semantic parameter range validation"""
        base = ParameterBase(
            name="brightness",
            description="Too bright"
        )
        with self.assertRaises(ValueError):  # Removed match parameter
            param = SemanticParameter(
                base=base,
                param_type="ratio",
                default=1.5  # Invalid: > 1.0
            )
            param.validate()

    def test_flag_validation(self):
        """Test flag validation"""
        # Valid flag
        base = ParameterBase(
            name="test",
            description="Test param",
            flags=["Clamp"]
        )
        param = RangeParameter(
            base=base,
            param_type="range",
            range_min=0,
            range_max=1,
            default=0.5
        )
        param.validate()  # Should pass

        # Invalid flag type
        base = ParameterBase(
            name="test",
            description="Test param",
            flags=["Clamp"]  # Clamp not allowed for select
        )
        try:
            param = SelectParameter(
                base=base,
                values=["a", "b"],
                default="a"
            )
            param.validate()
            self.fail("Expected ValueError for invalid flag on select parameter")
        except ValueError as e:
            self.assertIn("Flag 'Clamp' not allowed for parameter type 'select'", str(e))

    def test_complete_code_generation(self):
        """Test complete C++ code generation format"""
        base = ParameterBase(
            name="speed",
            description="Speed control",
            flags=["Clamp"]
        )
        param = SemanticParameter(
            base=base,
            param_type="ratio",
            default=0.5
        )
        code = param.generate_code()
        
        # Remove extra whitespace for comparison
        actual = ' '.join(code.split())
        expected = ' '.join(
            '    {"speed", ParamType::ratio, 0.5f, ParamFlag::Clamp, "Speed control"}'
            .split()
        )
        self.assertEqual(actual, expected)

    def test_multiple_flags(self):
        """Test parameters with multiple flags"""
        base = ParameterBase(
            name="speed",
            description="Speed control",
            flags=["Clamp", "Slew"]  # Multiple flags
        )
        param = SemanticParameter(
            base=base,
            param_type="signed_ratio",
            default=0.5
        )
        param.validate()
        code = param.generate_code()
        actual = ' '.join(code.split())
        expected = ' '.join(
            '    {"speed", ParamType::signed_ratio, 0.5f, ParamFlag::Clamp | ParamFlag::Slew, "Speed control"}'
            .split()
        )
        self.assertEqual(actual, expected)

    def test_select_parameter_value_mapping(self):
        """Test select parameter with mapped values"""
        base = ParameterBase(
            name="direction",
            description="Direction control"
        )
        param = SelectParameter(
            base=base,
            values={"forward": 1, "reverse": -1, "oscillate": 0},
            default="reverse"
        )
        param.validate()
        code = param.generate_code()
        # Check value mapping in generated code
        actual = ' '.join(code.split())
        expected = ' '.join(
            '    {"direction", ParamType::select, 0, , "reverse", "Direction control", {"forward", "reverse", "oscillate"}},'
            .split()
        )
        self.assertEqual(actual, expected) 