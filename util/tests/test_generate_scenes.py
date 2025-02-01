import unittest
import tempfile
import os
import yaml
from util.generate_scenes import process_scene, process_scenes
from util.parameters import (
    Parameter,
    PARAM_TYPES,
    create_parameter
)
import pytest

def load_yaml_param(yaml_str):
    """Helper to load YAML parameter definition"""
    return yaml.safe_load(yaml_str)

class TestSceneGenerator(unittest.TestCase):
    def test_parameter_types(self):
        """Test that parameter types are correctly handled"""
        yaml_str = """
        name: Test Scene
        parameters:
            ratio_param:
                type: ratio
                default: 0.5
                description: A ratio parameter
        """
        yaml_data = load_yaml_param(yaml_str)
        param = create_parameter("ratio_param", yaml_data['parameters']['ratio_param'])
        
        self.assertEqual(param.base.name, "ratio_param")
        self.assertEqual(param.param_type, "ratio")
        self.assertEqual(param.default, 0.5)
        self.assertEqual(param.base.description, "A ratio parameter")
        self.assertEqual(param.base.flags, None)

    def test_range_parameter(self):
        """Test parameter with range values"""
        yaml_str = """
        name: Test Scene
        parameters:
            count:
                type: count
                range: [0, 100]
                default: 50
                description: Count parameter
        """
        yaml_data = load_yaml_param(yaml_str)
        param = create_parameter("count", yaml_data['parameters']['count'])
        
        self.assertEqual(param.range_min, 0)
        self.assertEqual(param.range_max, 100)
        self.assertEqual(param.default, 50)

    def test_semantic_types(self):
        """Test semantic types with fixed ranges from spec"""
        yaml_str = """
        name: Test Scene
        parameters:
            speed:
                type: signed_ratio
                default: 0.5
                flags: [clamp]
                description: Controls animation speed
            brightness:
                type: ratio
                default: 0.8
                flags: [wrap]
                description: Overall LED brightness
        """
        yaml_data = load_yaml_param(yaml_str)
        
        speed_param = create_parameter('speed', yaml_data['parameters']['speed'])
        self.assertEqual(speed_param.param_type, 'signed_ratio')
        self.assertEqual(speed_param.default, 0.5)
        self.assertEqual(speed_param.base.flags, ['Clamp'])
        
        bright_param = create_parameter('brightness', yaml_data['parameters']['brightness'])
        self.assertEqual(bright_param.param_type, 'ratio')
        self.assertEqual(bright_param.default, 0.8)
        self.assertEqual(bright_param.base.flags, ['Wrap'])

    def test_choice_types(self):
        """Test select and switch parameters"""
        yaml_str = """
        name: Test Scene
        parameters:
            mode:
                type: select
                values: ['sphere', 'fountain', 'cascade']
                default: sphere
                description: Animation mode
            active:
                type: switch
                default: true
                description: Enable animation
        """
        yaml_data = load_yaml_param(yaml_str)
        
        # Test select parameter
        select_param = create_parameter('mode', yaml_data['parameters']['mode'])
        self.assertEqual(select_param.param_type, 'select')
        self.assertEqual(select_param.values, ['sphere', 'fountain', 'cascade'])
        self.assertEqual(select_param.default, 'sphere')
        
        # Test switch parameter
        switch_param = create_parameter('active', yaml_data['parameters']['active'])
        self.assertEqual(switch_param.param_type, 'switch')
        self.assertEqual(switch_param.default, True)

    def test_resource_types(self):
        """Test palette and bitmap parameters"""
        yaml_str = """
        name: Test Scene
        parameters:
            colors:
                type: palette
                default: rainbow
                description: Color scheme
            texture:
                type: bitmap
                default: stars
                description: Particle texture
        """
        yaml_data = load_yaml_param(yaml_str)
        
        # Test palette parameter
        palette_param = create_parameter('colors', yaml_data['parameters']['colors'])
        self.assertEqual(palette_param.param_type, 'palette')
        self.assertEqual(palette_param.default, 'rainbow')
        
        # Test bitmap parameter
        bitmap_param = create_parameter('texture', yaml_data['parameters']['texture'])
        self.assertEqual(bitmap_param.param_type, 'bitmap')
        self.assertEqual(bitmap_param.default, 'stars')

    def test_parameter_flags(self):
        """Test parameter flags are correctly handled"""
        yaml_str = """
        name: Test Scene
        parameters:
            speed:
                type: signed_ratio
                default: 0.5
                flags: [clamp, slew]
                description: Speed with flags
            brightness:
                type: ratio
                default: 1.0
                flags: [wrap]
                description: Brightness with wrap
        """
        yaml_data = load_yaml_param(yaml_str)
        
        speed_param = create_parameter('speed', yaml_data['parameters']['speed'])
        self.assertEqual(speed_param.base.flags, ['Clamp', 'Slew'])
        
        bright_param = create_parameter('brightness', yaml_data['parameters']['brightness'])
        self.assertEqual(bright_param.base.flags, ['Wrap'])

    def test_validation_errors(self):
        """Test parameter validation error cases"""
        test_cases = [
            ({
                'type': 'invalid_type'
            }, "Unknown parameter type 'invalid_type'"),
            ({
                'type': 'range',
                'default': 0
            }, "Parameter .* of type range requires range"),
            ({
                'type': 'select'
            }, "Parameter .* of type select requires 'values' field"),
        ]
        
        for param_data, expected_error in test_cases:
            with self.assertRaisesRegex(ValueError, expected_error):
                create_parameter('test', param_data)

    def test_basic_types(self):
        """Test basic types with ranges from spec"""
        yaml_str = """
        name: Test Scene
        parameters:
            particles:
                type: count
                range: [10, 1000]
                default: 100
                description: Number of particles
            scale:
                type: range
                range: [0.1, 10.0]
                default: 1.0
                description: Scale factor
        """
        yaml_data = load_yaml_param(yaml_str)
        
        count_param = create_parameter('particles', yaml_data['parameters']['particles'])
        self.assertEqual(count_param.param_type, 'count')
        self.assertEqual(count_param.range_min, 10)
        self.assertEqual(count_param.range_max, 1000)
        self.assertEqual(count_param.default, 100)
        
        range_param = create_parameter('scale', yaml_data['parameters']['scale'])
        self.assertEqual(range_param.param_type, 'range')
        self.assertEqual(range_param.range_min, 0.1)
        self.assertEqual(range_param.range_max, 10.0)
        self.assertEqual(range_param.default, 1.0)

    def test_select_type(self):
        """Test select type from spec"""
        yaml_str = """
        name: Test Scene
        parameters:
            pattern:
                type: select
                values: ["sphere", "fountain", "cascade"]
                default: sphere
                description: Animation pattern selection
        """
        yaml_data = load_yaml_param(yaml_str)
        
        select_param = create_parameter('pattern', yaml_data['parameters']['pattern'])
        self.assertEqual(select_param.param_type, 'select')
        self.assertEqual(select_param.values, ["sphere", "fountain", "cascade"])
        self.assertEqual(select_param.default, "sphere")

def test_generate_scenes():
    """Test scene parameter generation"""
    yaml_data = {
        'parameters': {
            'speed': {
                'type': 'ratio',
                'default': 0.5,
                'description': 'Animation speed'
            }
        }
    }
    
    header = process_scene('test_scene', yaml_data)
    assert 'ParamType::ratio' in header
    assert '"speed"' in header
    assert '0.5f' in header
    assert '"Animation speed"' in header

def test_scene_metadata():
    """Test scene metadata handling"""
    yaml_data = {
        'name': 'Test Scene',
        'description': 'A test scene',
        'parameters': {
            'speed': {
                'type': 'ratio',
                'default': 0.5
            }
        }
    }
    
    header = process_scene('test', yaml_data)
    assert '// A test scene' in header

def test_missing_scene_name():
    """Test error when scene name is missing"""
    yaml_data = {
        'parameters': {}
    }
    
    with pytest.raises(ValueError, match="missing required 'name' field"):
        process_scene('test', yaml_data)
