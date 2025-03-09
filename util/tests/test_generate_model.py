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

if __name__ == '__main__':
    unittest.main() 