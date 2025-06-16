#!/usr/bin/env python3

import os
import sys
import unittest
import tempfile
from pathlib import Path
import json
import yaml

# Add project root to Python path
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
sys.path.insert(0, project_root)

from util.generate_model import (
    Point3D,
    Neighbor,
    LedGroup,
    FaceType,
    Face,
    LED,
    ModelDefinition,
    DodecaModel
)

SAMPLE_MODEL_YAML = """
model:
  name: TestModel
  version: "1.0.0"
  description: "Test model for unit tests"

geometry:
  shape: Dodecahedron
  num_faces: 2
  edge_length_mm: 50.0
  radius_mm: 100.0

face_types:
  pentagon:
    num_leds: 104
    num_sides: 5
    groups:
      middle: [0]
      ring0: [1, 2, 3, 4, 5]
      ring1: [6, 7, 8, 9, 10]

faces:
  - id: 0
    type: pentagon
    rotation: 0
  - id: 1
    type: pentagon
    rotation: 3

hardware:
  pcb:
    pick_and_place_file: "test_pnp.csv"
    led_designator_prefix: "LED"
  led:
    type: WS2812B
    color_order: GRB
    diameter_mm: 1.6
    spacing_mm: 5.0
  power:
    max_current_per_led_ma: 20
    avg_current_per_led_ma: 10
"""

class TestPoint3D(unittest.TestCase):
    def test_distance_calculation(self):
        p1 = Point3D(0, 0, 0)
        p2 = Point3D(3, 4, 0)
        self.assertEqual(p1.distance_to(p2), 5.0)

        p3 = Point3D(1, 1, 1)
        p4 = Point3D(2, 2, 2)
        self.assertAlmostEqual(p3.distance_to(p4), 1.7320508075688772)  # sqrt(3)

class TestLedGroup(unittest.TestCase):
    def test_led_group_creation(self):
        group = LedGroup("test", [1, 2, 3])
        self.assertEqual(group.name, "test")
        self.assertEqual(group.led_indices, [1, 2, 3])

class TestFaceType(unittest.TestCase):
    def test_face_type_creation(self):
        face_type = FaceType(
            name="pentagon",
            num_leds=104,
            num_sides=5,
            edge_length_mm=50.0
        )
        self.assertEqual(face_type.name, "pentagon")
        self.assertEqual(face_type.num_leds, 104)
        self.assertEqual(face_type.num_sides, 5)
        self.assertEqual(face_type.edge_length_mm, 50.0)
        self.assertEqual(len(face_type.groups), 0)

    def test_face_type_with_groups(self):
        face_type = FaceType(
            name="pentagon",
            num_leds=104,
            num_sides=5,
            edge_length_mm=50.0
        )
        group = LedGroup("test", [1, 2, 3])
        face_type.groups["test"] = group
        self.assertEqual(len(face_type.groups), 1)
        self.assertEqual(face_type.groups["test"].led_indices, [1, 2, 3])

class TestModelDefinition(unittest.TestCase):
    def setUp(self):
        # Create a temporary YAML file
        self.temp_dir = tempfile.mkdtemp()
        self.yaml_path = os.path.join(self.temp_dir, "test_model.yaml")
        with open(self.yaml_path, "w") as f:
            f.write(SAMPLE_MODEL_YAML)

    def test_model_definition_loading(self):
        model_def = ModelDefinition(self.yaml_path)
        
        # Check model metadata
        self.assertEqual(model_def.model["name"], "TestModel")
        self.assertEqual(model_def.model["version"], "1.0.0")
        
        # Check geometry
        self.assertEqual(model_def.geometry["shape"], "Dodecahedron")
        self.assertEqual(model_def.geometry["num_faces"], 2)
        
        # Check face types
        self.assertEqual(len(model_def.face_types), 1)
        face_type = model_def.face_types["pentagon"]
        self.assertEqual(face_type.num_leds, 104)
        self.assertEqual(face_type.num_sides, 5)
        
        # Check LED groups
        self.assertEqual(len(face_type.groups), 3)
        self.assertEqual(len(face_type.groups["ring0"].led_indices), 5)
        
        # Check faces
        self.assertEqual(len(model_def.faces), 2)
        self.assertEqual(model_def.faces[0].rotation, 0)
        self.assertEqual(model_def.faces[1].rotation, 3)

        # Check hardware config
        self.assertEqual(model_def.hardware["led"]["type"], "WS2812B")
        self.assertEqual(model_def.hardware["pcb"]["led_designator_prefix"], "LED")

class TestDodecaModel(unittest.TestCase):
    def setUp(self):
        # Create temporary files
        self.temp_dir = tempfile.mkdtemp()
        self.yaml_path = os.path.join(self.temp_dir, "test_model.yaml")
        with open(self.yaml_path, "w") as f:
            f.write(SAMPLE_MODEL_YAML)

        # Create a simple PnP file for testing
        self.pnp_path = os.path.join(self.temp_dir, "test_pnp.csv")
        pnp_content = (
            "Designator\tMid X\tMid Y\n"
            "LED1\t0mm\t0mm\n"
            "LED2\t10mm\t0mm\n"
            "LED3\t0mm\t10mm\n"
        )
        with open(self.pnp_path, "w") as f:
            f.write(pnp_content)

    def test_model_generation(self):
        model_def = ModelDefinition(self.yaml_path)
        model = DodecaModel(model_def)
        
        # Test model loading and generation
        model.load_pcb_data(self.pnp_path)
        model.generate_model()
        
        # Check that LEDs were generated for each face
        expected_led_count = 3 * len(model_def.faces)  # 3 LEDs per face * 2 faces
        self.assertEqual(len(model_def.leds), expected_led_count)
        
        # Check that neighbors were calculated
        self.assertIsNotNone(model_def.leds[0].neighbors)
        
        # Test JSON export
        json_file = os.path.join(self.temp_dir, "test_output.json")
        with open(json_file, "w") as f:
            model.export_json(f)
        
        # Verify JSON content
        with open(json_file) as f:
            data = json.load(f)
            self.assertEqual(data["model"]["name"], "TestModel")
            self.assertEqual(len(data["points"]), expected_led_count)

        # Test C++ header export
        header_file = os.path.join(self.temp_dir, "test_output.h")
        with open(header_file, "w") as f:
            model.export_cpp_header(f)
        
        # Verify header content
        with open(header_file) as f:
            content = f.read()
            self.assertIn("namespace PixelTheater", content)
            self.assertIn("namespace Models", content)
            self.assertIn("struct TestModel", content)
            self.assertIn(f"static constexpr size_t LED_COUNT = {expected_led_count}", content)
            
            # Check for new format elements
            self.assertIn("static constexpr std::array<FaceTypeData,", content)
            self.assertIn("FACE_TYPES", content)
            self.assertIn("static constexpr std::array<FaceData, FACE_COUNT> FACES", content)
            self.assertIn("static constexpr PointData POINTS[]", content)
            self.assertIn("static constexpr NeighborData NEIGHBORS[]", content)

    def tearDown(self):
        # Clean up temporary files
        import shutil
        shutil.rmtree(self.temp_dir)

class TestFaceRemapping(unittest.TestCase):
    
    def create_test_yaml(self, faces_config):
        """Helper to create a test YAML file with the given faces configuration"""
        test_model = {
            'model': {
                'name': 'TestModel',
                'version': '1.0.0',
                'description': 'Test model for remapping',
                'author': 'Test'
            },
            'geometry': {
                'shape': 'Dodecahedron',
                'num_faces': 12,
                'edge_length_mm': 60.0,
                'radius_mm': 130.0
            },
            'face_types': {
                'pentagon': {
                    'num_leds': 135,
                    'num_sides': 5
                }
            },
            'faces': faces_config,
            'hardware': {
                'pcb': {
                    'pick_and_place_file': 'dummy.csv'
                }
            }
        }
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.yaml', delete=False) as f:
            yaml.dump(test_model, f)
            return f.name

    def test_face_without_remapping(self):
        """Test that faces without remap_to use their own ID"""
        faces_config = [
            {'id': 0, 'type': 'pentagon', 'rotation': 1},
            {'id': 1, 'type': 'pentagon', 'rotation': 2}
        ]
        
        yaml_path = self.create_test_yaml(faces_config)
        try:
            model_def = ModelDefinition(yaml_path)
            
            # Face without remap_to should use its own ID
            face_0 = model_def.faces[0]
            self.assertEqual(face_0.id, 0)
            self.assertIsNone(face_0.remap_to)
            self.assertEqual(face_0.get_geometric_id(), 0)
            
        finally:
            os.unlink(yaml_path)

    def test_face_with_remapping(self):
        """Test that faces with remap_to use the remapped ID for geometry"""
        faces_config = [
            {'id': 0, 'type': 'pentagon', 'remap_to': 2, 'rotation': 1},
            {'id': 1, 'type': 'pentagon', 'rotation': 2},
            {'id': 2, 'type': 'pentagon', 'remap_to': 0, 'rotation': 3}
        ]
        
        yaml_path = self.create_test_yaml(faces_config)
        try:
            model_def = ModelDefinition(yaml_path)
            
            # Face 0 should remap to geometric position 2
            face_0 = model_def.faces[0]
            self.assertEqual(face_0.id, 0)  # Logical ID preserved
            self.assertEqual(face_0.remap_to, 2)
            self.assertEqual(face_0.get_geometric_id(), 2)  # Uses remapped geometry
            
            # Face 1 should use its own position
            face_1 = model_def.faces[1]
            self.assertEqual(face_1.id, 1)
            self.assertIsNone(face_1.remap_to)
            self.assertEqual(face_1.get_geometric_id(), 1)
            
            # Face 2 should remap to geometric position 0
            face_2 = model_def.faces[2]
            self.assertEqual(face_2.id, 2)  # Logical ID preserved
            self.assertEqual(face_2.remap_to, 0)
            self.assertEqual(face_2.get_geometric_id(), 0)  # Uses remapped geometry
            
        finally:
            os.unlink(yaml_path)

    def test_remapping_validation(self):
        """Test that remapping values are reasonable"""
        faces_config = [
            {'id': 0, 'type': 'pentagon', 'remap_to': 11, 'rotation': 1}  # Valid - top face
        ]
        
        yaml_path = self.create_test_yaml(faces_config)
        try:
            model_def = ModelDefinition(yaml_path)
            face_0 = model_def.faces[0]
            
            # Should accept valid face IDs (0-11 for dodecahedron)
            self.assertEqual(face_0.get_geometric_id(), 11)
            
        finally:
            os.unlink(yaml_path)

def test_face_remapping_in_generated_model():
    """Test that face remapping works correctly in the generated model"""
    # Create a test YAML with face remapping
    test_yaml = """
model:
  name: TestModel
  version: "1.0"
  description: "Test model with face remapping"

geometry:
  shape: Test
  num_faces: 4
  edge_length_mm: 60.0

face_types:
  test:
    num_leds: 10
    num_sides: 3

faces:
  - id: 0
    type: test
    rotation: 0
    remap_to: 2  # Face 0 should be at geometric position 2
  - id: 1
    type: test
    rotation: 0
    # No remap_to, should be at geometric position 1
  - id: 2
    type: test
    rotation: 0
    remap_to: 0  # Face 2 should be at geometric position 0
  - id: 3
    type: test
    rotation: 0
    # No remap_to, should be at geometric position 3

hardware:
  pcb:
    pick_and_place_file: "test.csv"
  led:
    type: WS2812B
  power:
    max_current_per_led_ma: 20
"""
    
    with tempfile.NamedTemporaryFile(mode='w', suffix='.yaml', delete=False) as f:
        f.write(test_yaml)
        yaml_path = f.name
    
    try:
        # Load the model definition
        model_def = ModelDefinition(yaml_path)
        
        # Check that faces have correct geometric IDs
        face_by_id = {face.id: face for face in model_def.faces}
        
        # Face 0 should have geometric_id = 2 (remap_to: 2)
        assert face_by_id[0].get_geometric_id() == 2, f"Face 0 should have geometric_id 2, got {face_by_id[0].get_geometric_id()}"
        
        # Face 1 should have geometric_id = 1 (no remap_to)
        assert face_by_id[1].get_geometric_id() == 1, f"Face 1 should have geometric_id 1, got {face_by_id[1].get_geometric_id()}"
        
        # Face 2 should have geometric_id = 0 (remap_to: 0)
        assert face_by_id[2].get_geometric_id() == 0, f"Face 2 should have geometric_id 0, got {face_by_id[2].get_geometric_id()}"
        
        # Face 3 should have geometric_id = 3 (no remap_to)
        assert face_by_id[3].get_geometric_id() == 3, f"Face 3 should have geometric_id 3, got {face_by_id[3].get_geometric_id()}"
        
        print("✓ Face remapping geometric IDs are correct")
        
    finally:
        os.unlink(yaml_path)

if __name__ == '__main__':
    # Run the face remapping test
    test_face_remapping_in_generated_model()
    
    # Run unittest tests
    unittest.main() 