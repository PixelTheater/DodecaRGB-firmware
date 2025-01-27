import unittest
import tempfile
import os
from util.generate_scenes import generate_scene_header, process_scenes

class TestSceneGenerator(unittest.TestCase):
    def test_parameter_generation(self):
        yaml_data = {
            "name": "Test Scene",
            "parameters": {
                "speed": {
                    "type": "float",
                    "range": [-1.0, 1.0],
                    "default": 0.5
                }
            }
        }
        
        header = generate_scene_header("test_scene", yaml_data)
        
        # Check generated code
        self.assertIn("struct TestSceneParameters", header)
        self.assertIn('Parameter<float> speed', header)
        self.assertIn('-1.0, 1.0', header)
        self.assertIn('0.5', header)

    def test_invalid_yaml(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            # Create invalid YAML file
            with open(os.path.join(tmpdir, "invalid.yaml"), "w") as f:
                f.write("invalid: [\n")
            
            with self.assertRaises(yaml.YAMLError):
                process_scenes(tmpdir, tmpdir) 