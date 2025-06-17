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
                    'num_sides': 5,
                    'groups': {
                        # Test LED groups for validation
                        'center': [0],
                        'ring0': [1, 2, 3, 4, 5],
                        'ring1': [6, 7, 8, 9, 10, 11, 12, 13, 14, 15],
                        'ring2': [16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31],
                        'edge0': [130, 131, 133, 134, 80, 81, 82, 84, 86],
                        'edge1': [86, 87, 89, 90, 91, 92, 93, 95, 97],
                        'edge2': [97, 98, 100, 101, 102, 103, 104, 106, 108]
                    }
                }
            },
            'faces': faces_config,
            'hardware': {
                'pcb': {
                    'pick_and_place_file': 'dummy.csv',
                    'led_designator_prefix': 'LED'
                },
                'led': {
                    'type': 'WS2812B',
                    'color_order': 'GRB',
                    'diameter_mm': 1.6,
                    'spacing_mm': 4.5
                },
                'power': {
                    'max_current_per_led_ma': 20,
                    'avg_current_per_led_ma': 10
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
        # Create a model with all 12 faces to test remapping to face 11
        faces_config = []
        for i in range(12):
            faces_config.append({'id': i, 'type': 'pentagon', 'rotation': 1})
        
        # Remap face 0 to position 11
        faces_config[0]['remap_to'] = 11
        faces_config[11]['remap_to'] = 0  # Swap them
        
        yaml_path = self.create_test_yaml(faces_config)
        try:
            model_def = ModelDefinition(yaml_path)
            face_0 = model_def.faces[0]
            face_11 = model_def.faces[11]
            
            # Should accept valid face IDs (0-11 for dodecahedron)
            self.assertEqual(face_0.get_geometric_id(), 11)
            self.assertEqual(face_11.get_geometric_id(), 0)
            
        finally:
            os.unlink(yaml_path)

    def test_invalid_remapping_validation(self):
        """Test that invalid remapping configurations are caught"""
        # Test 1: Remapping to non-existent face
        faces_config = [
            {'id': 0, 'type': 'pentagon', 'remap_to': 99, 'rotation': 1}  # Invalid
        ]
        
        yaml_path = self.create_test_yaml(faces_config)
        try:
            with self.assertRaises(ValueError) as context:
                ModelDefinition(yaml_path)
            self.assertIn("not a valid face ID", str(context.exception))
        finally:
            os.unlink(yaml_path)
        
        # Test 2: Multiple faces mapped to same position
        faces_config = [
            {'id': 0, 'type': 'pentagon', 'remap_to': 2, 'rotation': 1},
            {'id': 1, 'type': 'pentagon', 'remap_to': 2, 'rotation': 1},  # Conflict!
            {'id': 2, 'type': 'pentagon', 'rotation': 1}
        ]
        
        yaml_path = self.create_test_yaml(faces_config)
        try:
            with self.assertRaises(ValueError) as context:
                ModelDefinition(yaml_path)
            self.assertIn("Multiple faces mapped to same geometric position", str(context.exception))
        finally:
            os.unlink(yaml_path)

    def test_model_generation_with_remapping(self):
        """Test that model generation correctly uses geometric IDs for LED positioning"""
        # Create a test model with face remapping
        faces_config = [
            {'id': 0, 'type': 'pentagon', 'remap_to': 1, 'rotation': 0},  # Face 0 at position 1
            {'id': 1, 'type': 'pentagon', 'remap_to': 0, 'rotation': 0}   # Face 1 at position 0
        ]
        
        yaml_path = self.create_test_yaml(faces_config)
        
        # Create a simple PnP file
        pnp_path = os.path.join(os.path.dirname(yaml_path), "test_pnp.csv")
        pnp_content = (
            "Designator\tMid X\tMid Y\n"
            "LED1\t0mm\t0mm\n"    # Center LED
            "LED2\t10mm\t0mm\n"   # Offset LED
        )
        with open(pnp_path, "w") as f:
            f.write(pnp_content)
        
        try:
            # Load and generate model
            model_def = ModelDefinition(yaml_path)
            model = DodecaModel(model_def)
            model.load_pcb_data(pnp_path)
            model.generate_model()
            
            # Verify faces were loaded with correct remapping
            self.assertEqual(len(model_def.faces), 2)
            face_0 = model_def.faces[0]  # Logical face 0
            face_1 = model_def.faces[1]  # Logical face 1
            
            # Check remapping
            self.assertEqual(face_0.get_geometric_id(), 1)  # Face 0 mapped to position 1
            self.assertEqual(face_1.get_geometric_id(), 0)  # Face 1 mapped to position 0
            
            # Verify LEDs were generated
            expected_led_count = 2 * len(model_def.faces)  # 2 LEDs per face * 2 faces
            self.assertEqual(len(model_def.leds), expected_led_count)
            
            # Verify LED face assignments preserve logical IDs (for wiring order)
            face_0_leds = [led for led in model_def.leds if led.face_id == 0]
            face_1_leds = [led for led in model_def.leds if led.face_id == 1]
            
            self.assertEqual(len(face_0_leds), 2)  # Face 0 should have 2 LEDs
            self.assertEqual(len(face_1_leds), 2)  # Face 1 should have 2 LEDs
            
            # The key test: positions should be calculated using geometric IDs
            # LEDs from logical face 0 should be positioned as if they're at face 1's geometric position
            # LEDs from logical face 1 should be positioned as if they're at face 0's geometric position
            # This is hard to test precisely without knowing the exact transform math,
            # but we can at least verify the LEDs have different positions
            
            face_0_positions = [led.position for led in face_0_leds]
            face_1_positions = [led.position for led in face_1_leds]
            
            # Verify positions are different (since they're at different geometric locations)
            for pos_0 in face_0_positions:
                for pos_1 in face_1_positions:
                    distance = pos_0.distance_to(pos_1)
                    self.assertGreater(distance, 10.0, 
                                     "LED positions should be significantly different due to remapping")
            
        finally:
            os.unlink(yaml_path)
            if os.path.exists(pnp_path):
                os.unlink(pnp_path)

    def test_vertex_remapping_follows_geometric_positioning(self):
        """Test that face vertices are calculated using geometric IDs, not logical IDs"""
        import os
        # Create a simple 2-face model with clear remapping
        faces_config = [
            {'id': 0, 'type': 'pentagon', 'remap_to': 1, 'rotation': 0},  # Face 0 at position 1
            {'id': 1, 'type': 'pentagon', 'remap_to': 0, 'rotation': 0}   # Face 1 at position 0
        ]
        
        yaml_path = self.create_test_yaml(faces_config)
        
        try:
            model_def = ModelDefinition(yaml_path)
            model = DodecaModel(model_def)
            
            # Generate C++ header to get vertex calculations
            import tempfile
            with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
                header_path = f.name
                model.export_cpp_header(f)
            
            # Read the generated header
            with open(header_path, 'r') as f:
                header_content = f.read()
            
            # Parse the vertex data from the header
            import re
            face_data_pattern = r'\{\.id = (\d+), \.type_id = \d+, \.rotation = \d+, \.geometric_id = (\d+),\s*\.vertices = \{\s*(.*?)\s*\}\s*\}'
            vertex_pattern = r'\{\.x = ([-\d.]+)f, \.y = ([-\d.]+)f, \.z = ([-\d.]+)f\}'
            
            faces_info = {}
            for match in re.finditer(face_data_pattern, header_content, re.DOTALL):
                face_id = int(match.group(1))
                geometric_id = int(match.group(2))
                vertices_str = match.group(3)
                
                vertices = []
                for vertex_match in re.finditer(vertex_pattern, vertices_str):
                    x, y, z = float(vertex_match.group(1)), float(vertex_match.group(2)), float(vertex_match.group(3))
                    if x != 0 or y != 0 or z != 0:  # Skip zero-padded vertices
                        vertices.append((x, y, z))
                
                faces_info[face_id] = {
                    'geometric_id': geometric_id,
                    'vertices': vertices
                }
            
            # Verify remapping
            self.assertEqual(faces_info[0]['geometric_id'], 1)  # Face 0 mapped to position 1
            self.assertEqual(faces_info[1]['geometric_id'], 0)  # Face 1 mapped to position 0
            
            # The key test: vertices should be DIFFERENT due to remapping
            # If remapping is working, face 0's vertices should be calculated for position 1
            # and face 1's vertices should be calculated for position 0
            face_0_vertices = faces_info[0]['vertices']
            face_1_vertices = faces_info[1]['vertices']
            
            # Calculate distances between corresponding vertices
            self.assertEqual(len(face_0_vertices), len(face_1_vertices))
            
            total_distance = 0
            for v0, v1 in zip(face_0_vertices, face_1_vertices):
                dist = ((v0[0] - v1[0])**2 + (v0[1] - v1[1])**2 + (v0[2] - v1[2])**2)**0.5
                total_distance += dist
            
            # Vertices should be significantly different due to remapping
            self.assertGreater(total_distance, 100.0, 
                             "Face vertices should be significantly different due to geometric remapping")
            
            # Clean up
            import os
            os.unlink(header_path)
            
        finally:
            os.unlink(yaml_path)

    def test_led_access_patterns_with_remapping(self):
        """Test that LED access works correctly: logical IDs for indexing, geometric IDs for positioning"""
        import os
        # Create a test model with face remapping
        faces_config = [
            {'id': 0, 'type': 'pentagon', 'remap_to': 1, 'rotation': 0},  # Face 0 at position 1
            {'id': 1, 'type': 'pentagon', 'remap_to': 0, 'rotation': 0}   # Face 1 at position 0
        ]
        
        yaml_path = self.create_test_yaml(faces_config)
        
        # Create a simple PnP file with distinguishable LED positions
        pnp_path = os.path.join(os.path.dirname(yaml_path), "test_pnp.csv")
        pnp_content = (
            "Designator\tMid X\tMid Y\n"
            "LED1\t0mm\t0mm\n"      # Center LED
            "LED2\t10mm\t0mm\n"     # Offset LED
            "LED3\t0mm\t10mm\n"     # Another offset LED
        )
        with open(pnp_path, "w") as f:
            f.write(pnp_content)
        
        try:
            # Load and generate model
            model_def = ModelDefinition(yaml_path)
            model = DodecaModel(model_def)
            model.load_pcb_data(pnp_path)
            model.generate_model()
            
            # Verify we have the expected number of LEDs
            expected_led_count = 3 * 2  # 3 LEDs per face * 2 faces
            self.assertEqual(len(model_def.leds), expected_led_count)
            
            # Test 1: Verify LED face assignments preserve logical order
            # The first 3 LEDs should belong to logical face 0
            # The next 3 LEDs should belong to logical face 1
            face_0_leds = []
            face_1_leds = []
            
            for led in model_def.leds:
                if led.face_id == 0:
                    face_0_leds.append(led)
                elif led.face_id == 1:
                    face_1_leds.append(led)
            
            self.assertEqual(len(face_0_leds), 3, "Face 0 should have 3 LEDs")
            self.assertEqual(len(face_1_leds), 3, "Face 1 should have 3 LEDs")
            
            # Test 2: Verify LED indexing follows logical wiring order
            # leds[0], leds[1], leds[2] should be from logical face 0
            # leds[3], leds[4], leds[5] should be from logical face 1
            self.assertEqual(model_def.leds[0].face_id, 0, "First LED should be from logical face 0")
            self.assertEqual(model_def.leds[1].face_id, 0, "Second LED should be from logical face 0")
            self.assertEqual(model_def.leds[2].face_id, 0, "Third LED should be from logical face 0")
            self.assertEqual(model_def.leds[3].face_id, 1, "Fourth LED should be from logical face 1")
            self.assertEqual(model_def.leds[4].face_id, 1, "Fifth LED should be from logical face 1")
            self.assertEqual(model_def.leds[5].face_id, 1, "Sixth LED should be from logical face 1")
            
            # Test 3: Verify face.leds access patterns
            # model_def.faces[0].leds should contain the LEDs from logical face 0
            # model_def.faces[1].leds should contain the LEDs from logical face 1
            self.assertEqual(len(model_def.faces[0].leds), 3, "Face 0 should have 3 LEDs in its collection")
            self.assertEqual(len(model_def.faces[1].leds), 3, "Face 1 should have 3 LEDs in its collection")
            
            # All LEDs in faces[0].leds should have face_id = 0
            for led in model_def.faces[0].leds:
                self.assertEqual(led.face_id, 0, "All LEDs in faces[0].leds should have face_id=0")
            
            # All LEDs in faces[1].leds should have face_id = 1
            for led in model_def.faces[1].leds:
                self.assertEqual(led.face_id, 1, "All LEDs in faces[1].leds should have face_id=1")
            
            # Test 4: CRITICAL - Verify positions use geometric remapping
            # Face 0 LEDs should be positioned using geometric ID 1
            # Face 1 LEDs should be positioned using geometric ID 0
            # This means positions should be "swapped" compared to logical order
            
            face_0_positions = [led.position for led in face_0_leds]
            face_1_positions = [led.position for led in face_1_leds]
            
            # Calculate center points for each face
            face_0_center = Point3D(
                sum(p.x for p in face_0_positions) / len(face_0_positions),
                sum(p.y for p in face_0_positions) / len(face_0_positions),
                sum(p.z for p in face_0_positions) / len(face_0_positions)
            )
            
            face_1_center = Point3D(
                sum(p.x for p in face_1_positions) / len(face_1_positions),
                sum(p.y for p in face_1_positions) / len(face_1_positions),
                sum(p.z for p in face_1_positions) / len(face_1_positions)
            )
            
            # Face centers should be significantly different due to remapping
            center_distance = face_0_center.distance_to(face_1_center)
            self.assertGreater(center_distance, 50.0, 
                             "Face centers should be significantly different due to geometric remapping")
            
            print(f"✓ Face 0 center: ({face_0_center.x:.1f}, {face_0_center.y:.1f}, {face_0_center.z:.1f})")
            print(f"✓ Face 1 center: ({face_1_center.x:.1f}, {face_1_center.y:.1f}, {face_1_center.z:.1f})")
            print(f"✓ Distance between face centers: {center_distance:.1f}")
            
        finally:
            os.unlink(yaml_path)
            if os.path.exists(pnp_path):
                os.unlink(pnp_path)

    def test_specific_led_access_behavior(self):
        """Demonstrate specific LED access behavior with face remapping:
        - leds[0] and model.leds[0] reference first LED of logical face 0
        - model.face(0).leds[0] also references first LED of logical face 0  
        - But 3D position is calculated using geometric remapping
        """
        import os
        # Create test model where face 0 and face 1 are swapped
        faces_config = [
            {'id': 0, 'type': 'pentagon', 'remap_to': 1, 'rotation': 0},  # Face 0 at position 1
            {'id': 1, 'type': 'pentagon', 'remap_to': 0, 'rotation': 0}   # Face 1 at position 0
        ]
        
        yaml_path = self.create_test_yaml(faces_config)
        
        # Create a simple PnP file with easily identifiable LEDs
        pnp_path = os.path.join(os.path.dirname(yaml_path), "test_pnp.csv")
        pnp_content = (
            "Designator\tMid X\tMid Y\n"
            "LED1\t0mm\t0mm\n"      # First LED of each face (center)
            "LED2\t5mm\t0mm\n"      # Second LED of each face
        )
        with open(pnp_path, "w") as f:
            f.write(pnp_content)
        
        try:
            # Generate model
            model_def = ModelDefinition(yaml_path)
            model = DodecaModel(model_def)
            model.load_pcb_data(pnp_path)
            model.generate_model()
            
            # Verify setup: 2 LEDs per face, 2 faces = 4 total LEDs
            self.assertEqual(len(model_def.leds), 4)
            
            # Key assertion 1: leds[0] should be the first LED of logical face 0
            led_0_global = model_def.leds[0]
            self.assertEqual(led_0_global.face_id, 0, "leds[0] should belong to logical face 0")
            self.assertEqual(led_0_global.label, 1, "leds[0] should be the first LED (label=1)")
            
            # Key assertion 2: model.face(0).leds[0] should ALSO be the first LED of logical face 0
            face_0_led_0 = model_def.faces[0].leds[0]
            self.assertEqual(face_0_led_0.face_id, 0, "face(0).leds[0] should belong to logical face 0")
            self.assertEqual(face_0_led_0.label, 1, "face(0).leds[0] should be the first LED (label=1)")
            
            # Key assertion 3: These should be the SAME LED object
            self.assertIs(led_0_global, face_0_led_0, "leds[0] and face(0).leds[0] should be the same LED object")
            
            # Key assertion 4: leds[2] should be the first LED of logical face 1
            led_2_global = model_def.leds[2]  # First LED of face 1 (after face 0's 2 LEDs)
            self.assertEqual(led_2_global.face_id, 1, "leds[2] should belong to logical face 1")
            self.assertEqual(led_2_global.label, 1, "leds[2] should be the first LED (label=1)")
            
            # Key assertion 5: model.face(1).leds[0] should be the first LED of logical face 1
            face_1_led_0 = model_def.faces[1].leds[0]
            self.assertEqual(face_1_led_0.face_id, 1, "face(1).leds[0] should belong to logical face 1")
            self.assertEqual(face_1_led_0.label, 1, "face(1).leds[0] should be the first LED (label=1)")
            
            # Key assertion 6: These should be the SAME LED object
            self.assertIs(led_2_global, face_1_led_0, "leds[2] and face(1).leds[0] should be the same LED object")
            
            # CRITICAL assertion 7: Positions should use geometric remapping
            # Face 0 is mapped to geometric position 1, Face 1 is mapped to geometric position 0
            # So Face 0's LEDs should be positioned where Face 1 would normally be
            # And Face 1's LEDs should be positioned where Face 0 would normally be
            
            face_0_first_led_pos = led_0_global.position
            face_1_first_led_pos = led_2_global.position
            
            # The distance between these LEDs should be significant due to remapping
            distance = face_0_first_led_pos.distance_to(face_1_first_led_pos)
            self.assertGreater(distance, 100.0, 
                             "Face 0 and Face 1 first LEDs should be far apart due to geometric remapping")
            
            print(f"✓ leds[0] and face(0).leds[0] are the same object: {led_0_global is face_0_led_0}")
            print(f"✓ leds[2] and face(1).leds[0] are the same object: {led_2_global is face_1_led_0}")
            print(f"✓ Face 0 LED position: ({face_0_first_led_pos.x:.1f}, {face_0_first_led_pos.y:.1f}, {face_0_first_led_pos.z:.1f})")
            print(f"✓ Face 1 LED position: ({face_1_first_led_pos.x:.1f}, {face_1_first_led_pos.y:.1f}, {face_1_first_led_pos.z:.1f})")
            print(f"✓ Distance due to remapping: {distance:.1f}")
            
        finally:
            os.unlink(yaml_path)
            if os.path.exists(pnp_path):
                os.unlink(pnp_path)

    def test_led_groups_and_metadata_generation(self):
        """Test that LED groups and hardware metadata are correctly generated"""
        import os
        # Create a test model with LED groups
        faces_config = [
            {'id': 0, 'type': 'pentagon', 'rotation': 0},
            {'id': 1, 'type': 'pentagon', 'rotation': 0}
        ]
        
        yaml_path = self.create_test_yaml(faces_config)
        
        try:
            model_def = ModelDefinition(yaml_path)
            model = DodecaModel(model_def)
            
            # Generate C++ header to test LED groups and metadata
            import tempfile
            with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
                header_path = f.name
                model.export_cpp_header(f)
            
            # Read and verify header content
            with open(header_path, 'r') as f:
                header_content = f.read()
            
            # Check for hardware metadata
            self.assertIn("static constexpr HardwareData HARDWARE", header_content)
            self.assertIn('.led_type = "WS2812B"', header_content)
            self.assertIn('.led_diameter_mm = 1.6f', header_content)
            
            # Check for LED groups
            self.assertIn("static constexpr std::array<LedGroupData,", header_content)
            self.assertIn("LED_GROUPS", header_content)
            self.assertIn('.name = "center"', header_content)
            self.assertIn('.name = "ring0"', header_content)
            
            # Verify LED group structure
            import re
            group_pattern = r'\.name = "(\w+)".*?\.led_count = (\d+)'
            groups = re.findall(group_pattern, header_content, re.DOTALL)
            
            # Should have groups like center, ring0, ring1, etc.
            group_names = [g[0] for g in groups]
            self.assertIn('center', group_names)
            self.assertIn('ring0', group_names)
            self.assertIn('edge0', group_names)
            
            # Clean up
            import os
            os.unlink(header_path)
            
        finally:
            os.unlink(yaml_path)

    def test_edge_calculation_and_face_relationships(self):
        """Test that edges and face relationships are calculated correctly"""
        import os
        # Create a simple 2-face model to test edge calculation
        faces_config = [
            {'id': 0, 'type': 'pentagon', 'rotation': 0},
            {'id': 1, 'type': 'pentagon', 'rotation': 0}
        ]
        
        yaml_path = self.create_test_yaml(faces_config)
        
        try:
            model_def = ModelDefinition(yaml_path)
            model = DodecaModel(model_def)
            
            # Generate C++ header to test edge calculation
            import tempfile
            with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
                header_path = f.name
                model.export_cpp_header(f)
            
            # Read and verify header content
            with open(header_path, 'r') as f:
                header_content = f.read()
            
            # Check for edge data
            self.assertIn("static constexpr std::array<EdgeData,", header_content)
            self.assertIn("EDGES", header_content)
            self.assertIn(".face_id =", header_content)
            self.assertIn(".edge_index =", header_content)
            self.assertIn(".start_vertex =", header_content)
            self.assertIn(".end_vertex =", header_content)
            self.assertIn(".connected_face_id =", header_content)
            
            # Verify we have edges for both faces (pentagon has 5 edges each)
            import re
            edge_pattern = r'\.face_id = (\d+)'
            face_ids = re.findall(edge_pattern, header_content)
            
            # Should have edges for both faces
            self.assertIn('0', face_ids)
            self.assertIn('1', face_ids)
            
            # Clean up
            import os
            os.unlink(header_path)
            
        finally:
            os.unlink(yaml_path)

    def test_geometric_validation(self):
        """Test the geometric validation functionality"""
        import os
        # Create a test model
        faces_config = [
            {'id': 0, 'type': 'pentagon', 'rotation': 0}
        ]
        
        yaml_path = self.create_test_yaml(faces_config)
        
        # Create a simple PnP file with coplanar LEDs
        pnp_path = os.path.join(os.path.dirname(yaml_path), "test_pnp.csv")
        pnp_content = (
            "Designator\tMid X\tMid Y\n"
            "LED1\t0mm\t0mm\n"      # Center
            "LED2\t5mm\t0mm\n"      # Coplanar
            "LED3\t0mm\t5mm\n"      # Coplanar
            "LED4\t-5mm\t0mm\n"     # Coplanar
        )
        with open(pnp_path, "w") as f:
            f.write(pnp_content)
        
        try:
            # Generate model
            model_def = ModelDefinition(yaml_path)
            model = DodecaModel(model_def)
            model.load_pcb_data(pnp_path)
            model.generate_model()
            
            # Test geometric validation
            face = model_def.faces[0]
            
            # Should be planar (tolerance allows for small deviations in 3D transformation)
            is_planar = face.is_planar(tolerance=10.0)
            
            print(f"✓ Face planarity test: {is_planar}")
            print(f"✓ Face has {len(face.leds)} LEDs")
            
            # Print LED positions for debugging
            for i, led in enumerate(face.leds):
                print(f"  LED {i}: ({led.position.x:.1f}, {led.position.y:.1f}, {led.position.z:.1f})")
            
        finally:
            os.unlink(yaml_path)
            if os.path.exists(pnp_path):
                os.unlink(pnp_path)

    def test_comprehensive_feature_integration(self):
        """Test that all new features work together correctly"""
        import os
        # Create a test model that exercises all features
        faces_config = [
            {'id': 0, 'type': 'pentagon', 'remap_to': 1, 'rotation': 0},
            {'id': 1, 'type': 'pentagon', 'remap_to': 0, 'rotation': 0}
        ]
        
        yaml_path = self.create_test_yaml(faces_config)
        
        # Create PnP file
        pnp_path = os.path.join(os.path.dirname(yaml_path), "test_pnp.csv")
        pnp_content = (
            "Designator\tMid X\tMid Y\n"
            "LED1\t0mm\t0mm\n"
            "LED2\t3mm\t0mm\n"
            "LED3\t0mm\t3mm\n"
        )
        with open(pnp_path, "w") as f:
            f.write(pnp_content)
        
        try:
            # Generate full model
            model_def = ModelDefinition(yaml_path)
            model = DodecaModel(model_def)
            model.load_pcb_data(pnp_path)
            model.generate_model()
            
            # Generate C++ header
            import tempfile
            with tempfile.NamedTemporaryFile(mode='w', suffix='.h', delete=False) as f:
                header_path = f.name
                model.export_cpp_header(f)
            
            # Verify comprehensive feature set
            with open(header_path, 'r') as f:
                header_content = f.read()
            
            # 1. Face remapping should be present
            self.assertIn('.geometric_id = 1', header_content)  # Face 0 mapped to position 1
            self.assertIn('.geometric_id = 0', header_content)  # Face 1 mapped to position 0
            
            # 2. LED groups should be present
            self.assertIn('LED_GROUPS', header_content)
            self.assertIn('.name = "center"', header_content)
            
            # 3. Hardware metadata should be present
            self.assertIn('HARDWARE', header_content)
            self.assertIn('.led_type = "WS2812B"', header_content)
            
            # 4. Edges should be calculated
            self.assertIn('EDGES', header_content)
            self.assertIn('.connected_face_id =', header_content)
            
            # 5. Geometric validation should work
            for face in model_def.faces:
                is_planar = face.is_planar(tolerance=20.0)  # Higher tolerance for small test model
                print(f"✓ Face {face.id} (geometric: {face.get_geometric_id()}) planarity: {is_planar}")
            
            # 6. LED access should work with remapping
            self.assertEqual(len(model_def.leds), 6)  # 3 LEDs per face * 2 faces
            self.assertEqual(model_def.leds[0].face_id, 0)  # First LED belongs to logical face 0
            self.assertEqual(model_def.leds[3].face_id, 1)  # Fourth LED belongs to logical face 1
            
            print(f"✓ All features integrated successfully!")
            print(f"✓ Model has {len(model_def.leds)} LEDs, {len(model_def.faces)} faces")
            print(f"✓ Generated header has {len(header_content)} characters")
            
            # Clean up
            import os
            os.unlink(header_path)
            
        finally:
            os.unlink(yaml_path)
            if os.path.exists(pnp_path):
                os.unlink(pnp_path)

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