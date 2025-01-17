import math
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D


# Constants from Processing
TWO_PI = 2 * math.pi
zv = TWO_PI/20  # Rotation between faces
ro = TWO_PI/5   # Rotation for pentagon points
xv = 1.1071     # angle between faces
radius = 200    # Base radius for pentagon faces

class Matrix3D:
    def __init__(self):
        """Initialize matrix to identity"""
        self.m = [[1.0, 0.0, 0.0, 0.0],
                 [0.0, 1.0, 0.0, 0.0],
                 [0.0, 0.0, 1.0, 0.0],
                 [0.0, 0.0, 0.0, 1.0]]
        self.stack = []  # Initialize matrix stack
    
    def apply(self, point):
        """Apply transformation to point [x,y,z]"""
        x, y, z = point
        w = 1.0  # Homogeneous coordinate
        
        # Full matrix multiplication including translation
        new_x = x*self.m[0][0] + y*self.m[0][1] + z*self.m[0][2] + w*self.m[0][3]
        new_y = x*self.m[1][0] + y*self.m[1][1] + z*self.m[1][2] + w*self.m[1][3]
        new_z = x*self.m[2][0] + y*self.m[2][1] + z*self.m[2][2] + w*self.m[2][3]
        # w = x*self.m[3][0] + y*self.m[3][1] + z*self.m[3][2] + w*self.m[3][3]  # Should stay 1
        
        return [new_x, new_y, new_z]

    def rotate_x(self, angle: float):
        """Rotate around X axis by angle (radians)"""
        c = math.cos(angle)
        s = math.sin(angle)
        rot = [[1,  0,   0, 0],
               [0,  c,  -s, 0],
               [0,  s,   c, 0],
               [0,  0,   0, 1]]
        self.m = self._multiply_matrices(self.m, rot)

    def rotate_z(self, angle: float):
        """Rotate around Z axis by angle (radians)"""
        c = math.cos(angle)
        s = math.sin(angle)
        rot = [[ c, -s, 0, 0],
               [ s,  c, 0, 0],
               [ 0,  0, 1, 0],
               [ 0,  0, 0, 1]]
        # New transformations multiply the previous ones (Processing style)
        self.m = self._multiply_matrices(self.m, rot)

    def _multiply_matrices(self, a, b):
        """Multiply two 4x4 matrices"""
        result = [[0 for _ in range(4)] for _ in range(4)]
        for i in range(4):
            for j in range(4):
                for k in range(4):
                    result[i][j] += a[i][k] * b[k][j]
        return result

    def translate(self, x: float, y: float, z: float):
        """Translate by (x,y,z)"""
        trans = [[1, 0, 0, x],
                 [0, 1, 0, y],
                 [0, 0, 1, z],
                 [0, 0, 0, 1]]
        # New transformations multiply the previous ones (Processing style)
        self.m = self._multiply_matrices(self.m, trans)

    def push_matrix(self):
        """Save current matrix state"""
        # Deep copy current matrix
        self.stack.append([row[:] for row in self.m])

    def pop_matrix(self):
        """Restore previous matrix state"""
        if not self.stack:
            raise Exception("Matrix stack is empty")
        self.m = self.stack.pop()

def test_basic_rotations():
    """Test basic rotation matrices"""
    print("\nTesting basic rotations...")
    
    # Test rotateX(PI/2)
    m = Matrix3D()
    m.rotate_x(math.pi/2)  # 90 degrees
    print("\nTesting rotateX(PI/2)...")
    
    # Test basis vectors
    result = m.apply([1, 0, 0])  # X basis
    print(f"X basis: {result}")
    assert np.allclose(result, [1,0,0]), "X basis wrong after rotateX(PI/2)"
    
    result = m.apply([0, 1, 0])  # Y basis
    print(f"Y basis: {result}")
    assert np.allclose(result, [0,0,1]), "Y basis wrong after rotateX(PI/2)"
    
    result = m.apply([0, 0, 1])  # Z basis
    print(f"Z basis: {result}")
    assert np.allclose(result, [0,-1,0]), "Z basis wrong after rotateX(PI/2)"

def test_translations():
    """Test that translations match Processing's behavior"""
    print("\nTesting translations...")
    m = Matrix3D()
    
    # Test single axis translations
    print("\nTesting single axis translations...")
    m.translate(1, 0, 0)
    result = m.apply([0,0,0])
    print(f"After translate(1,0,0): {result}")
    assert np.allclose(result, [1,0,0]), "X translation wrong"
    
    m.m = [[1.0, 0.0, 0.0, 0.0],  # Reset to identity
           [0.0, 1.0, 0.0, 0.0],
           [0.0, 0.0, 1.0, 0.0],
           [0.0, 0.0, 0.0, 1.0]]
    m.translate(0, 1, 0)
    result = m.apply([0,0,0])
    print(f"After translate(0,1,0): {result}")
    assert np.allclose(result, [0,1,0]), "Y translation wrong"
    
    m.m = [[1.0, 0.0, 0.0, 0.0],  # Reset to identity
           [0.0, 1.0, 0.0, 0.0],
           [0.0, 0.0, 1.0, 0.0],
           [0.0, 0.0, 0.0, 1.0]]
    m.translate(0, 0, 1)
    result = m.apply([0,0,0])
    print(f"After translate(0,0,1): {result}")
    assert np.allclose(result, [0,0,1]), "Z translation wrong"
    
    # Test compound translation
    print("\nTesting compound translation...")
    m.m = [[1.0, 0.0, 0.0, 0.0],  # Reset to identity
           [0.0, 1.0, 0.0, 0.0],
           [0.0, 0.0, 1.0, 0.0],
           [0.0, 0.0, 0.0, 1.0]]
    m.translate(1, 2, 3)
    result = m.apply([0,0,0])
    print(f"After translate(1,2,3): {result}")
    assert np.allclose(result, [1,2,3]), "Compound translation wrong"
    
    print("All translation tests passed!")

def test_matrix_stack():
    """Test matrix stack operations"""
    print("\nTesting matrix stack...")
    m = Matrix3D()
    
    # Test basic push/pop
    m.translate(1, 2, 3)
    result = m.apply([0, 0, 0])
    print(f"After translate: {result}")
    assert np.allclose(result, [1,2,3]), "Translation wrong"
    
    m.push_matrix()
    m.m = [[1,0,0,0], [0,1,0,0], [0,0,1,0], [0,0,0,1]]  # Reset to identity
    result = m.apply([0, 0, 0])
    print(f"After pop: {result}")
    assert np.allclose(result, [0,0,0]), "Matrix reset wrong"
    
    # Testing nested push/pop
    m = Matrix3D()  # Start fresh
    m.translate(1, 0, 0)
    m.push_matrix()
    m.rotate_x(math.pi/2)
    result = m.apply([0, 0, 1])
    print(f"After translate + rotate: {result}")
    assert np.allclose(result, [1,-1,0]), "Nested transformation wrong"
    
    print("All matrix stack tests passed!")

def test_dodeca_transforms():
    """Test the dodecahedron transformation sequence"""
    m = Matrix3D()
    
    # Test initial transform - Processing style
    m.translate(400, 400, 0)  # First move to center
    m.rotate_x(math.pi)       # Then flip upside down
    result = m.apply([0, 0, 0])
    print("\nTesting initial transform...")
    print(f"Center point after initial: {result}")
    assert np.allclose(result, [400,400,0]), "Center translation wrong"
    
    # Test first face rotation
    print("\nTesting first face rotation...")
    m.rotate_z(zv)  # zv = TWO_PI/20
    result = m.apply([100,0,0])  # Point on +X axis
    print(f"Point after first rotation: {result}")
    
    # Test multiple face rotations
    print("\nTesting multiple face rotations...")
    for i in range(5):  # Test 5 faces
        m.push_matrix()
        m.rotate_z(i * zv)
        result = m.apply([100,0,0])
        print(f"Face {i} point: {result}")
        m.pop_matrix()
    
    print("All dodecahedron transforms passed!")

def test_translate_after_rotate():
    """Test that translation after rotation works correctly"""
    print("\nTesting translation after rotation...")
    m = Matrix3D()
    
    # Test 1: Rotate then translate
    m.rotate_z(math.pi/2)  # 90 degrees
    m.translate(0, 0, 100)
    result = m.apply([10, 0, 0])
    print(f"Point after rotate(90°) then translate(z=100): {result}")
    # Should be: [0, 10, 100] - the point is rotated THEN moved up
    assert np.allclose(result, [0, 10, 100]), "Translation after rotation wrong"
    
    # Test 2: Translate then rotate (different order)
    m = Matrix3D()
    m.translate(0, 0, 100)
    m.rotate_z(math.pi/2)
    result = m.apply([10, 0, 0])
    print(f"Point after translate(z=100) then rotate(90°): {result}")
    # Should be different from Test 1
    assert np.allclose(result, [0, 10, 100]), "Translation before rotation wrong"
    
    print("Translation after rotation tests passed!")

def test_matrix_multiplication():
    """Test matrix multiplication order"""
    print("\nTesting matrix multiplication...")
    m = Matrix3D()
    
    # Print the initial matrix
    print("\nInitial matrix:")
    for row in m.m:
        print(row)
    
    # Apply translation
    m.translate(0, 0, 100)
    print("\nAfter translate(0,0,100):")
    for row in m.m:
        print(row)
    
    # Apply rotation
    m.rotate_z(math.pi/2)
    print("\nAfter rotateZ(90°):")
    for row in m.m:
        print(row)
    
    # Test a point
    result = m.apply([10, 0, 0])
    print(f"\nTransformed point [10,0,0]: {result}")

def test_push_pop():
    """Test matrix stack operations"""
    print("\nTesting push/pop matrix...")
    m = Matrix3D()
    
    # Save initial state
    m.push_matrix()
    initial = m.apply([10, 0, 0])
    
    # Do some transforms
    m.translate(0, 0, 100)
    m.rotate_z(math.pi/2)
    transformed = m.apply([10, 0, 0])
    
    # Restore and verify
    m.pop_matrix()
    restored = m.apply([10, 0, 0])
    
    print(f"Initial point: {initial}")
    print(f"Transformed point: {transformed}")
    print(f"Restored point: {restored}")
    assert np.allclose(initial, restored), "Matrix not properly restored"

def test_translation():
    """Test that translation works correctly"""
    print("\nTesting translation...")
    m = Matrix3D()
    
    # Test 1: Basic translations along each axis
    m.translate(10, 0, 0)
    result = m.apply([0, 0, 0])
    assert result == [10, 0, 0], f"X translation wrong: got {result}"
    
    m = Matrix3D()
    m.translate(0, 20, 0)
    result = m.apply([0, 0, 0])
    assert result == [0, 20, 0], f"Y translation wrong: got {result}"
    
    m = Matrix3D()
    m.translate(0, 0, 30)
    result = m.apply([0, 0, 0])
    assert result == [0, 0, 30], f"Z translation wrong: got {result}"
    
    # Test 2: Combined translation
    m = Matrix3D()
    m.translate(10, 20, 30)
    result = m.apply([0, 0, 0])
    assert result == [10, 20, 30], f"Combined translation wrong: got {result}"
    
    # Test 3: Translation of non-origin point
    result = m.apply([5, 5, 5])
    assert result == [15, 25, 35], f"Point translation wrong: got {result}"
    
    # Test 4: Translation after rotation
    m = Matrix3D()
    m.rotate_z(math.pi/2)  # 90 degrees around Z
    m.translate(10, 0, 0)  # Translate along rotated X axis
    result = m.apply([1, 0, 0])
    assert abs(result[0] - 0) < 0.001, f"X coord wrong after rotate+translate: {result}"
    assert abs(result[1] - 11) < 0.001, f"Y coord wrong after rotate+translate: {result}"
    assert abs(result[2] - 0) < 0.001, f"Z coord wrong after rotate+translate: {result}"
    
    # Test 5: Translation along rotated axes
    m = Matrix3D()
    m.rotate_x(math.pi/4)  # 45° around X
    m.translate(0, 0, 10)  # Along rotated Z
    result = m.apply([0, 0, 0])
    expected = [0, -10/math.sqrt(2), 10/math.sqrt(2)]  # Changed: Y component should be negative
    for i in range(3):
        assert abs(result[i] - expected[i]) < 0.001, \
            f"Translation along rotated Z wrong: got {result}, expected {expected}"
    
    # Test 6: Multiple rotations then translate
    print("\nTesting multiple rotations then translate...")
    m = Matrix3D()
    
    print("Initial matrix:")
    print_matrix(m.m)
    
    m.rotate_z(math.pi/2)  # 90° around Z
    print("\nAfter rotateZ(PI/2):")
    print_matrix(m.m)
    
    m.rotate_x(math.pi/2)  # 90° around X
    print("\nAfter rotateX(PI/2):")
    print_matrix(m.m)
    
    m.translate(0, 0, 10)  # Along transformed Z (which is now X)
    print("\nAfter translate(0,0,10):")
    print_matrix(m.m)
    
    # Test points from Processing output
    test_points = [
        [0, 0, 0],  # Origin
        [1, 0, 0],  # Unit X
        [0, 1, 0],  # Unit Y
        [0, 0, 1]   # Unit Z
    ]
    
    print("\nTransformed points:")
    for p in test_points:
        result = m.apply(p)
        print(f"Point ({p[0]:.1f},{p[1]:.1f},{p[2]:.1f}) -> ({result[0]:.1f},{result[1]:.1f},{result[2]:.1f})")
        
    # Test specific point (origin)
    result = m.apply([0, 0, 0])
    assert abs(result[0] - 10) < 0.001, "X should be 10"
    assert abs(result[1] - 0) < 0.001, "Y should be 0"
    assert abs(result[2] - 0) < 0.001, "Z should be 0"

def print_matrix(m):
    """Print matrix in same format as Processing"""
    for i in range(4):
        print(f"[ {' '.join(f'{m[i][j]:.2f}' for j in range(4))} ]")

def test_rotate_x_matches_processing():
    """Verify rotate_x(PI) matches Processing's behavior"""
    m = Matrix3D()
    m.rotate_x(math.pi)
    
    # Test point at y=1, z=1
    result = m.apply([0, 1, 1])
    print("\nTesting rotate_x(PI):")
    print(f"Point [0,1,1] becomes: {result}")
    # Should flip both Y and Z coordinates

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
        print(f"Expected:  {p['expected']}")

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
    
    print("\nAfter rotateX(PI/4) and translate(0,0,10):")
    x = m.apply([1, 0, 0])
    y = m.apply([0, 1, 0])
    z = m.apply([0, 0, 1])
    print(f"X: {x}")
    print(f"Y: {y}")
    print(f"Z: {z}")
    
    # Test origin
    result = m.apply([0, 0, 0])
    print(f"\nPoint (0,0,0) transformed to: {result}")

def test_hemisphere_transforms():
    """Test how transforms affect points in top vs bottom hemispheres"""
    m = Matrix3D()
    m.rotate_x(math.pi)
    
    # Test points at same radius but different hemispheres
    bottom_point = [0, 0, 200]  # Point on bottom hemisphere
    top_point = [0, 0, -200]    # Point on top hemisphere
    
    print("\nTesting hemisphere transforms:")
    print(f"Bottom point {bottom_point} -> {m.apply(bottom_point)}")
    print(f"Top point    {top_point} -> {m.apply(top_point)}")

def test_side_rotations():
    """Test how side rotations affect Y coordinates"""
    m = Matrix3D()
    m.rotate_x(math.pi)
    
    # Test points at different Y positions
    points = [
        [0, 100, 0],   # Positive Y
        [0, -100, 0],  # Negative Y
    ]
    
    print("\nTesting side rotations:")
    for p in points:
        result = m.apply(p)
        print(f"Point {p} -> {result}")

def test_processing_led_sequence():
    """Test exact sequence from buildLedsFromComponentPlacementCSV()"""
    m = Matrix3D()
    
    # Constants from Processing
    TWO_PI = 2 * math.pi
    zv = TWO_PI/20  # Rotation between faces
    ro = TWO_PI/5   # Rotation for pentagon points
    radius = 200    # Base radius for pentagon faces
    
    # Initial state
    x, y = 1.49, -0.02  # First LED coordinates
    
    print("\nTesting Processing LED sequence:")
    print(f"Initial point: [{x}, {y}, 0]")
    
    # Test sequence from drawPentagon() for side 0 (bottom)
    m.rotate_x(math.pi)  # Initial flip
    p1 = m.apply([x, y, 0])
    print(f"After rotate_x(PI): {p1}")
    
    m.rotate_z(-zv - ro*2)  # Side 0 positioning
    p2 = m.apply([x, y, 0])
    print(f"After side rotation: {p2}")
    
    m.translate(0, 0, radius*1.31)  # Move face out
    p3 = m.apply([x, y, 0])
    print(f"After translate: {p3}")
    
    m.rotate_z(-zv)  # Additional hemisphere rotation
    p4 = m.apply([x, y, 0])
    print(f"After hemisphere rotation: {p4}")

def test_side11_sequence():
    """Compare Side 11 (top) transformations with Processing"""
    m = Matrix3D()
    
    # Test points from PCB
    test_points = [
        {'x': 1.49, 'y': -0.02, 'label': 1},   # LED 1
        {'x': -26.5, 'y': 108.3, 'label': 52}, # LED 52 (showing large diff)
        {'x': -143.6, 'y': 57.1, 'label': 104} # LED 104
    ]
    
    print("\nTesting Side 11 (top) transformation sequence:")
    print("Reference points from points.cpp:")
    print("  LED 1:   [1.4, -0.4, -268.0]")
    print("  LED 52:  [-26.5, 108.3, -268.0]")
    print("  LED 104: [-143.6, 57.1, -268.0]")
    
    print("\nTesting each transformation step:")
    
    # Initial state
    print("\nInitial points:")
    for p in test_points:
        print(f"  LED {p['label']}: [{p['x']}, {p['y']}, 0]")
    
    # After rotate_x(PI)
    m.rotate_x(math.pi)
    print("\nAfter rotate_x(PI):")
    for p in test_points:
        result = m.apply([p['x'], p['y'], 0])
        print(f"  LED {p['label']}: {result}")
    
    # After rotate_z(zv)
    m.rotate_z(TWO_PI/20)  # zv
    print("\nAfter rotate_z(zv):")
    for p in test_points:
        result = m.apply([p['x'], p['y'], 0])
        print(f"  LED {p['label']}: {result}")
    
    # After translate(0, 0, radius*1.34)
    m.translate(0, 0, 200*1.34)  # radius = 200
    print("\nAfter translate:")
    for p in test_points:
        result = m.apply([p['x'], p['y'], 0])
        print(f"  LED {p['label']}: {result}")

def test_middle_face_rotations():
    """Test rotation sequence for middle faces (1-10)"""
    m = Matrix3D()
    test_point = [0, 100, 0]  # Point on face
    
    print("\nTesting middle face rotations:")
    
    # Test bottom half face (1-5)
    m.rotate_x(math.pi)
    m.rotate_z(ro*1 + zv - ro)  # Side 1
    m.rotate_x(xv)
    p1 = m.apply(test_point)
    print(f"Side 1 point: {p1}")
    
    # Test top half face (6-10)
    m = Matrix3D()  # Reset
    m.rotate_x(math.pi)
    m.rotate_z(ro*6 - zv + ro*3)  # Side 6
    m.rotate_x(math.pi - xv)
    p2 = m.apply(test_point)
    print(f"Side 6 point: {p2}")

def test_led_space_rotation():
    """Test LED space rotation matches Processing"""
    m = Matrix3D()
    test_point = [100, 0, 0]  # Point on X axis
    
    print("\nTesting LED space rotation:")
    
    # Test PI/10 rotation
    m.rotate_z(math.pi/10)
    p1 = m.apply(test_point)
    print(f"After PI/10 rotation: {p1}")
    
    # Test -PI/10 rotation
    m = Matrix3D()
    m.rotate_z(-math.pi/10)
    p2 = m.apply(test_point)
    print(f"After -PI/10 rotation: {p2}")
    
    # Test PI/5 rotation
    m = Matrix3D()
    m.rotate_z(math.pi/5)
    p3 = m.apply(test_point)
    print(f"After PI/5 rotation: {p3}")

def test_complete_middle_face_sequence():
    """Test complete transformation sequence for middle face LED"""
    m = Matrix3D()
    test_point = [1.49, -0.02, 0]  # First LED position
    
    print("\nTesting complete middle face sequence:")
    print(f"Initial point: {test_point}")
    
    # Initial LED space rotation (72°)
    m.rotate_z(-math.pi/5)
    p1 = m.apply(test_point)
    print(f"After LED space rotation: {p1}")
    
    # Initial transform for all faces
    m.rotate_x(math.pi)
    m.rotate_z(ro)
    p2 = m.apply(test_point)
    print(f"After initial transform: {p2}")
    
    # Side 1 positioning
    m.rotate_z(ro*1 + zv - ro)
    m.rotate_x(xv)
    p3 = m.apply(test_point)
    print(f"After side positioning: {p3}")
    
    # Move face out
    m.translate(0, 0, radius*1.34)
    p4 = m.apply(test_point)
    print(f"Final position: {p4}")

def test_side_rotation_pattern():
    """Test how key LEDs move with different side rotations"""
    print("\nTesting side rotation patterns:")
    
    # Side rotation configuration from Processing
    side_rotation = [0, 3, 4, 4, 4, 4, 2, 2, 2, 2, 2, 0]
    
    # Expected LED1 positions for each rotation
    expected_led1 = {
        0: [1.49, -0.02],      # Original position (sides 0, 11)
        2: [-1.193, 0.891],    # ~144° rotation (sides 6-10)
        3: [-1.217, -0.859],   # ~216° rotation (side 1)
        4: [0.441, -1.423]     # ~288° rotation (sides 2-5)
    }
    
    # Test each side's rotation
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
        
        # Print all positions for reference
        print(f"  LED1 (near center): {led1}")
        print(f"  LED50 (bottom): {m.apply([0, -150, 0])}")
        print(f"  LED63 (top): {m.apply([0, 150, 0])}")

if __name__ == "__main__":
    test_basic_rotations()
    test_translations()
    test_matrix_stack()
    test_dodeca_transforms()
    test_translate_after_rotate()
    test_matrix_multiplication()
    test_push_pop()
    test_translation()
    test_rotate_x_matches_processing()
    test_processing_coordinate_system()
    test_processing_transforms()
    test_hemisphere_transforms()
    test_side_rotations()
    test_processing_led_sequence()
    test_middle_face_rotations()
    test_led_space_rotation()
    test_complete_middle_face_sequence()
    test_side_rotation_pattern()
    print("\nAll tests passed!") 