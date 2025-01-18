import math
import numpy as np
from matrix3d import Matrix3D

# Constants used in tests
TWO_PI = 2 * math.pi
zv = TWO_PI/20  # Rotation between faces
ro = TWO_PI/5   # Rotation for pentagon points
xv = 1.1071     # angle between faces
radius = 100    # Base radius for pentagon faces (matches Processing)

def print_matrix(m):
    """Print matrix in same format as Processing"""
    for i in range(4):
        print(f"[ {' '.join(f'{m[i][j]:.2f}' for j in range(4))} ]")

def print_test_section(name):
    """Print a consistently formatted test section header"""
    print(f"\n=== {name} ===")

def format_vector(v, precision=3):
    """Format a vector with consistent precision"""
    return f"[{', '.join(f'{x:.{precision}f}' for x in v)}]"

def test_basic_rotations():
    """Test basic rotation matrices"""
    print("Testing basic rotations...")
    
    m = Matrix3D()
    m.rotate_x(math.pi/2)  # 90 degrees
    
    # Test basis vectors
    result = m.apply([1, 0, 0])
    print(f"  X basis: {format_vector(result)}")
    assert np.allclose(result, [1,0,0]), "X basis wrong after rotateX(PI/2)"
    
    result = m.apply([0, 1, 0])
    print(f"  Y basis: {format_vector(result)}")
    assert np.allclose(result, [0,0,1]), "Y basis wrong after rotateX(PI/2)"
    
    result = m.apply([0, 0, 1])
    print(f"  Z basis: {format_vector(result)}")
    assert np.allclose(result, [0,-1,0]), "Z basis wrong after rotateX(PI/2)"

def test_translations():
    """Test that translations match Processing's behavior"""
    print_test_section("Testing translations...")
    m = Matrix3D()
    
    # Test single axis translations
    m.translate(1, 0, 0)
    result = m.apply([0,0,0])
    assert np.allclose(result, [1,0,0]), "X translation wrong"
    
    m = Matrix3D()
    m.translate(0, 1, 0)
    result = m.apply([0,0,0])
    assert np.allclose(result, [0,1,0]), "Y translation wrong"
    
    m = Matrix3D()
    m.translate(0, 0, 1)
    result = m.apply([0,0,0])
    assert np.allclose(result, [0,0,1]), "Z translation wrong"

def test_matrix_stack():
    """Test matrix stack operations"""
    print_test_section("Testing matrix stack...")
    m = Matrix3D()
    
    # Test basic push/pop
    m.translate(1, 2, 3)
    m.push_matrix()
    original = m.apply([0, 0, 0])
    
    m.m = [[1,0,0,0], [0,1,0,0], [0,0,1,0], [0,0,0,1]]  # Reset
    m.pop_matrix()
    restored = m.apply([0, 0, 0])
    
    assert np.allclose(original, restored), "Matrix stack restore failed"

def test_processing_coordinate_system():
    """Verify our coordinate system matches Processing's"""
    m = Matrix3D()
    m.rotate_x(math.pi)
    
    # Test points
    points = [
        {'input': [0, 1, 1], 'expected': [0, -1, -1]},    # Point in +Y, +Z should flip both
        {'input': [1, 0, -1], 'expected': [1, 0, 1]},     # Point in +X, -Z should flip Z
    ]
    
    print("\nTesting Processing coordinate system:")
    for p in points:
        result = m.apply(p['input'])
        print(f"Point {p['input']} -> {result}")
        assert np.allclose(result, p['expected']), f"Expected {p['expected']}, got {result}"

def test_processing_transforms():
    """Match Processing's test_transforms.pde behavior exactly"""
    m = Matrix3D()
    
    print("\nTesting Processing transforms:")
    
    # Initial state
    print("\nInitial basis vectors:")
    x = m.apply([1, 0, 0])
    y = m.apply([0, 1, 0])
    z = m.apply([0, 0, 1])
    print(f"X: {x}")
    print(f"Y: {y}")
    print(f"Z: {z}")
    
    # Test sequence
    m.rotate_x(math.pi/4)  # 45° around X
    m.translate(0, 0, 10)  # Translate along rotated Z
    
    result = m.apply([0, 0, 0])
    expected = [0, -10/math.sqrt(2), 10/math.sqrt(2)]
    assert np.allclose(result, expected), f"Expected {expected}, got {result}"

def test_led_coordinate_adjustment():
    """Test the initial LED coordinate adjustments"""
    print_test_section("Testing LED coordinate adjustments...")
    x, y = 1.49, -0.02
    
    # Apply adjustments
    x += 0.2
    y -= 55.884
    
    print(f"Original: [{1.49}, {-0.02}]")
    print(f"Adjusted: [{x}, {y}]")
    
    assert np.isclose(x, 1.69)
    assert np.isclose(y, -55.904)

def test_led_rotation():
    """Test the LED rotation by PI/10"""
    print_test_section("Testing LED rotation...")
    m = Matrix3D()
    
    # Initial point
    x, y = 1.69, -55.904
    print(f"Before rotation: [{x}, {y}]")
    
    # Apply rotation
    m.rotate_z(math.pi/10)
    result = m.apply([x, y, 0])
    print(f"After rotation: [{result[0]:.1f}, {result[1]:.1f}, {result[2]:.1f}]")

def test_dodeca_transforms():
    """Test the dodecahedron transformation sequence"""
    m = Matrix3D()
    
    # Test initial transform - Processing style
    m.translate(400, 400, 0)  # First move to center
    m.rotate_x(math.pi)       # Then flip upside down
    result = m.apply([0, 0, 0])
    assert np.allclose(result, [400,400,0]), "Center translation wrong"
    
    # Test first face rotation
    m.rotate_z(zv)  # zv = TWO_PI/20
    result = m.apply([100,0,0])  # Point on +X axis
    print(f"Point after first rotation: {result}")

def test_translate_after_rotate():
    """Test that translation after rotation works correctly"""
    print_test_section("Testing translation after rotation...")
    m = Matrix3D()
    
    # Test 1: Rotate then translate
    m.rotate_z(math.pi/2)  # 90 degrees
    m.translate(0, 0, 100)
    result = m.apply([10, 0, 0])
    print(f"Point after rotate(90°) then translate(z=100): {result}")
    assert np.allclose(result, [0, 10, 100]), "Translation after rotation wrong"
    
    # Test 2: Translate then rotate (different order)
    m = Matrix3D()
    m.translate(0, 0, 100)
    m.rotate_z(math.pi/2)
    result = m.apply([10, 0, 0])
    print(f"Point after translate(z=100) then rotate(90°): {result}")
    assert np.allclose(result, [0, 10, 100]), "Translation before rotation wrong"

def test_matrix_multiplication():
    """Test matrix multiplication order"""
    print_test_section("Testing matrix multiplication...")
    m = Matrix3D()
    
    # Print the initial matrix
    print("\nInitial matrix:")
    print_matrix(m.m)
    
    # Apply translation
    m.translate(0, 0, 100)
    print("\nAfter translate(0,0,100):")
    print_matrix(m.m)
    
    # Apply rotation
    m.rotate_z(math.pi/2)
    print("\nAfter rotateZ(90°):")
    print_matrix(m.m)
    
    # Test a point
    result = m.apply([10, 0, 0])
    print(f"\nTransformed point [10,0,0]: {result}")
    assert np.allclose(result, [0, 10, 100]), "Matrix multiplication order wrong"

def test_hemisphere_transforms():
    """Test how transforms affect points in top vs bottom hemispheres"""
    m = Matrix3D()
    m.rotate_x(math.pi)
    
    # Test points at same radius but different hemispheres
    bottom_point = [0, 0, 200]  # Point on bottom hemisphere
    top_point = [0, 0, -200]    # Point on top hemisphere
    
    print("\nTesting hemisphere transforms:")
    bottom_result = m.apply(bottom_point)
    top_result = m.apply(top_point)
    print(f"Bottom point {bottom_point} -> {bottom_result}")
    print(f"Top point    {top_point} -> {top_result}")
    
    assert np.allclose(bottom_result[2], -200), "Bottom hemisphere Z wrong"
    assert np.allclose(top_result[2], 200), "Top hemisphere Z wrong"

def test_side_rotations():
    """Test how side rotations affect Y coordinates"""
    m = Matrix3D()
    m.rotate_x(math.pi)
    
    # Test points at different Y positions
    test_points = [
        {'input': [0, 100, 0], 'expected': [0, -100, 0]},   # Positive Y should flip
        {'input': [0, -100, 0], 'expected': [0, 100, 0]},   # Negative Y should flip
    ]
    
    print("\nTesting side rotations:")
    for p in test_points:
        result = m.apply(p['input'])
        print(f"Point {p['input']} -> {result}")
        assert np.allclose(result, p['expected'], atol=1e-10), \
            f"Expected {p['expected']}, got {result}"

def test_side_rotation_pattern():
    """Test how key LEDs move with different side rotations"""
    print_test_section("Testing side rotation patterns:")
    
    # Side rotation configuration from Processing
    side_rotation = [0, 3, 4, 4, 4, 4, 2, 2, 2, 2, 2, 0]
    
    # Expected LED1 positions for each rotation
    expected_led1 = {
        0: [1.49, -0.02],      # Original position (sides 0, 11)
        2: [-1.193, 0.891],    # ~144° rotation (sides 6-10)
        3: [-1.217, -0.859],   # ~216° rotation (side 1)
        4: [0.441, -1.423]     # ~288° rotation (sides 2-5)
    }
    
    for side in range(12):
        rotation = side_rotation[side]
        print(f"\nSide {side} (rotation {rotation}):")
        
        m = Matrix3D()
        m.rotate_z(ro * rotation)
        
        # Transform LED1 and verify position
        led1 = m.apply([1.49, -0.02, 0])
        expected = expected_led1[rotation]
        assert abs(led1[0] - expected[0]) < 0.01, f"Side {side} LED1 X mismatch"
        assert abs(led1[1] - expected[1]) < 0.01, f"Side {side} LED1 Y mismatch"


def test_middle_face_rotations():
    """Test rotation sequence for middle faces (1-10)"""
    m = Matrix3D()
    test_point = [0, 100, 0]  # Point on face
    
    print("\nTesting middle face rotations:")
    
    # Test bottom half face (1-5)
    m.rotate_x(math.pi)
    m.rotate_z(ro*1 + zv - ro)  # Side 1
    m.rotate_x(xv)
    result = m.apply(test_point)
    expected = [-13.821, -42.537, -89.441]  # Values from Processing
    assert np.allclose(result, expected, atol=0.001), \
        f"Bottom half face wrong. Expected {expected}, got {result}"
    
    # Test top half face (6-10)
    m = Matrix3D()
    m.rotate_x(math.pi)
    m.rotate_z(ro*6 - zv + ro*3)  # Side 6
    m.rotate_x(math.pi - xv)
    result = m.apply(test_point)
    expected = [-44.726, 0.000, -89.441]  # From Processing
    assert np.allclose(result, expected, atol=0.001), \
        f"Top half face wrong. Expected {expected}, got {result}"

def test_led_space_rotation():
    """Test LED space rotation matches Processing"""
    m = Matrix3D()
    test_point = [100, 0, 0]  # Point on X axis
    
    print("\nTesting LED space rotation:")
    
    # Test PI/10 rotation
    m.rotate_z(math.pi/10)
    result = m.apply(test_point)
    expected = [95.11, 30.90, 0]  # From Processing
    assert np.allclose(result, expected, atol=0.1), \
        f"PI/10 rotation wrong. Expected {expected}, got {result}"

def test_complete_middle_face_sequence():
    """Test complete transformation sequence for middle face LED"""
    m = Matrix3D()
    test_point = [1.49, -0.02, 0]  # First LED position
    
    print("\nTesting complete middle face sequence:")
    
    # Initial LED space rotation (-72°)
    m.rotate_z(-math.pi/5)  # -72° = -PI/2.5 = -PI/5
    result = m.apply(test_point)
    result = format_point(result)
    print(f"After LED space rotation: {result}")
    expected = [1.194, -0.892, 0.000]  # From Processing
    assert np.allclose(result, expected, atol=0.001), \
        f"LED space rotation wrong. Expected {expected}, got {result}"
    
    # Complete sequence
    m = Matrix3D()
    m.rotate_x(math.pi)
    m.rotate_z(ro)
    m.rotate_z(ro*1 + zv - ro)
    m.rotate_x(xv)
    m.translate(0, 0, radius*1.34)
    
    result = m.apply(test_point)
    result = format_point(result)
    print(f"After complete sequence: {result}")
    expected = [119.859, -1.490, -59.915]  # From Processing
    assert np.allclose(result, expected, atol=0.001), \
        f"Complete sequence wrong. Expected {expected}, got {result}"

def test_led_translation():
    """Test the LED translation with scale"""
    print_test_section("Testing LED translation...")
    m = Matrix3D()
    scale = 5.15
    x, y = 1.69, -55.904
    
    print(f"Before translation: [{x}, {y}]")
    m.translate(x*scale, y*scale, 0)
    result = m.apply([0, 0, 0])
    print(f"After translation: [{result[0]:.1f}, {result[1]:.1f}, {result[2]:.1f}]")
    
    expected = [x*scale, y*scale, 0]
    assert np.allclose(result, expected, atol=0.1), \
        f"LED translation wrong. Expected {expected}, got {result}"

def test_side1_led_sequence():
    """Test the complete transformation sequence for Side 1"""
    print_test_section("Testing Side 1 LED sequence...")
    
    # Initial LED position
    x, y = 1.49, -0.02
    point = [x, y, 0]
    print(f"Initial point: {point}")
    
    # 1. LED space rotation (PI/10)
    m = Matrix3D()
    m.rotate_z(math.pi/10)
    result = m.apply(point)
    result = format_point(result)
    print(f"After LED space rotation: {result}")
    expected = [1.423, 0.441, 0.000]  # From Processing
    assert np.allclose(result, expected, atol=0.001), \
        f"LED space rotation wrong. Expected {expected}, got {result}"
    
    # 2. Initial transform
    m = Matrix3D()
    m.rotate_x(math.pi)  # Flip upside down
    m.rotate_z(-3 * ro)  # Side 1 rotation
    result = m.apply(point)
    result = format_point(result)
    print(f"After initial transform: {result}")
    expected = [-1.194, -0.892, 0.000]  # From Processing
    assert np.allclose(result, expected, atol=0.001), \
        f"Initial transform wrong. Expected {expected}, got {result}"
    
    # 3. Side positioning
    m = Matrix3D()
    m.rotate_x(math.pi)  # Flip upside down
    m.rotate_z(zv)      # Side 1 positioning (zv = TWO_PI/20)
    m.rotate_x(xv)      # Side tilt
    result = m.apply(point)
    result = format_point(result)
    print(f"After side positioning: {result}")
    expected = [1.420, -0.452, 0.018]  # From Processing
    assert np.allclose(result, expected, atol=0.001), \
        f"Side positioning wrong. Expected {expected}, got {result}"

def is_close_to_zero(value, atol=1e-10):
    """Helper to check if a value is effectively zero"""
    return abs(value) < atol

def format_point(point):
    """Format a point with consistent precision, treating near-zero values as 0"""
    return [0.0 if is_close_to_zero(x) else round(x, 3) for x in point]

def test_all_side_transforms():
    """Test transformation sequences for all side types (top, middle, bottom)"""
    print_test_section("Testing all side transforms")
    test_point = [1.49, -0.02, 0]  # LED 1 position
    
    # Test top face (side 0)
    m = Matrix3D()
    m.rotate_x(math.pi)
    m.rotate_z(ro*0)  # No extra rotation
    result = m.apply(test_point)
    result = format_point(result)
    print(f"Side 0 (top): {result}")
    expected = [1.490, 0.020, 0.000]  # From Processing
    assert np.allclose(result, expected, atol=0.001), \
        f"Top face wrong. Expected {expected}, got {result}"
    
    # Test middle face (side 5)
    m = Matrix3D()
    m.rotate_x(math.pi)
    m.rotate_z(ro*5 + zv - ro)
    m.rotate_x(xv)
    result = m.apply(test_point)
    result = format_point(result)
    print(f"Side 5 (middle): {result}")
    expected = [0.869, 1.211, 0.018]  # From Processing
    assert np.allclose(result, expected, atol=0.001), \
        f"Middle face wrong. Expected {expected}, got {result}"
    
    # Test bottom face (side 11)
    m = Matrix3D()
    m.rotate_x(math.pi)
    m.rotate_z(ro*11)
    result = m.apply(test_point)
    result = format_point(result)
    print(f"Side 11 (bottom): {result}")
    expected = [0.479, -1.411, 0.000]  # From Processing
    assert np.allclose(result, expected, atol=0.001), \
        f"Bottom face wrong. Expected {expected}, got {result}"

if __name__ == "__main__":
    print("Matrix3D Test Suite")
    print("==================")
    
    # Basic operations
    print_test_section("Basic Matrix Operations")
    test_basic_rotations()
    test_translations()
    test_matrix_stack()
    
    # Processing compatibility
    print_test_section("Processing Compatibility")
    test_processing_coordinate_system()
    test_processing_transforms()
    
    # LED-specific transformations
    print_test_section("LED Transformations")
    test_led_coordinate_adjustment()
    test_led_rotation()
    test_led_translation()
    
    # Complex sequences
    print_test_section("Complex Transformation Sequences")
    test_dodeca_transforms()
    test_side1_led_sequence()
    
    # Additional tests
    print_test_section("Additional Tests")
    test_translate_after_rotate()
    test_matrix_multiplication()
    test_hemisphere_transforms()
    
    # Side-specific tests
    print_test_section("Side-Specific Tests")
    test_side_rotations()
    test_side_rotation_pattern()
    test_all_side_transforms()
    
    # Middle face tests
    print_test_section("Middle Face Tests")
    test_middle_face_rotations()
    test_led_space_rotation()
    test_complete_middle_face_sequence()
    
    print("\n✓ All tests passed!") 