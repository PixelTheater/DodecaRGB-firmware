import math
import numpy as np
from matrix3d import Matrix3D

def test_led_coordinate_adjustment():
    """Test the initial LED coordinate adjustments"""
    print("\nTesting LED coordinate adjustments...")
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
    print("\nTesting LED rotation...")
    m = Matrix3D()
    
    # Initial point
    x, y = 1.69, -55.904
    print(f"Before rotation: [{x}, {y}]")
    
    # Apply rotation
    m.rotate_z(math.pi/10)
    result = m.apply([x, y, 0])
    print(f"After rotation: [{result[0]:.1f}, {result[1]:.1f}, {result[2]:.1f}]")
    
    # Verify rotation results
    # Add expected values based on Processing output

def test_led_translation():
    """Test the LED translation with scale"""
    print("\nTesting LED translation...")
    m = Matrix3D()
    scale = 5.15
    x, y = 1.69, -55.904
    
    print(f"Before translation: [{x}, {y}]")
    m.translate(x*scale, y*scale, 0)
    result = m.apply([0, 0, 0])
    print(f"After translation: [{result[0]:.1f}, {result[1]:.1f}, {result[2]:.1f}]")
    
    # Verify translation results

def test_side1_led_sequence():
    """Test the complete transformation sequence for Side 1"""
    print("\nTesting Side 1 LED sequence...")
    m = Matrix3D()
    
    # Initial LED position
    x, y = 1.49, -0.02
    print(f"Initial point: [{x}, {y}, 0]")
    
    # 1. LED space rotation (PI/10)
    m.rotate_z(math.pi/10)
    result = m.apply([x, y, 0])
    print(f"After LED space rotation: [{result[0]:.1f}, {result[1]:.1f}, {result[2]:.1f}]")
    
    # 2. Initial transform
    m = Matrix3D()
    m.rotate_x(math.pi)  # Flip upside down
    m.rotate_z(ro)       # Global rotation
    result = m.apply([x, y, 0])
    print(f"After initial transform: [{result[0]:.1f}, {result[1]:.1f}, {result[2]:.1f}]")
    
    # 3. Side positioning (rotation 3)
    m = Matrix3D()
    m.rotate_z(3 * ro)  # Use rotation pattern from side_rotation[]
    m.rotate_x(xv)
    result = m.apply([x, y, 0])
    print(f"After side positioning: [{result[0]:.1f}, {result[1]:.1f}, {result[2]:.1f}]")
    
    # 4. Move face out
    m.translate(0, 0, radius * 1.31)
    result = m.apply([x, y, 0])
    print(f"Final position: [{result[0]:.1f}, {result[1]:.1f}, {result[2]:.1f}]")
    
    # Target from points.cpp
    print(f"Target position: [73.0, -228.7, 119.1]")

def print_matrix(m):
    """Print a 4x4 matrix in a readable format"""
    for i in range(4):
        print(f"[ {m[i][0]:6.2f} {m[i][1]:6.2f} {m[i][2]:6.2f} {m[i][3]:6.2f} ]")

def test_side1_processing_sequence():
    """Test Side 1 transformation sequence matching Processing"""
    print("\nTesting Side 1 Processing sequence...")
    m = Matrix3D()
    
    # Constants from Processing
    scale = 5.15
    radius = 200
    ro = math.pi * 2/5    # TWO_PI/5
    zv = math.pi * 2/20   # TWO_PI/20
    xv = 1.1071          # angle between faces
    
    # Initial LED position
    x, y = 1.49, -0.02
    print(f"Initial point: [{x}, {y}, 0]")
    
    # LED coordinate adjustments (from Processing)
    x += 0.2
    y -= 55.884
    print(f"After coordinate adjustment: [{x}, {y}, 0]")
    
    # Print each transformation step
    print("\nTransformation sequence:")
    
    # 1. LED space rotation
    m.rotate_z(math.pi/10)
    result = m.apply([x, y, 0])
    print("\nAfter LED rotation:")
    print_matrix(m.m)
    print(f"Point: [{result[0]:.1f}, {result[1]:.1f}, {result[2]:.1f}]")
    
    # 2. LED position
    m.translate(x*scale, y*scale, 0)
    result = m.apply([0, 0, 0])
    print("\nAfter LED translation:")
    print_matrix(m.m)
    print(f"Point: [{result[0]:.1f}, {result[1]:.1f}, {result[2]:.1f}]")
    
    # 3. Side positioning
    m.rotate_z(ro*(1)+zv-ro)
    result = m.apply([0, 0, 0])
    print("\nAfter side rotation:")
    print_matrix(m.m)
    print(f"Point: [{result[0]:.1f}, {result[1]:.1f}, {result[2]:.1f}]")
    
    # 4. Side tilt
    m.rotate_x(xv)
    result = m.apply([0, 0, 0])
    print("\nAfter side tilt:")
    print_matrix(m.m)
    print(f"Point: [{result[0]:.1f}, {result[1]:.1f}, {result[2]:.1f}]")
    
    # 5. Move face out
    m.translate(0, 0, radius*1.31)
    result = m.apply([0, 0, 0])
    print("\nFinal matrix:")
    print_matrix(m.m)
    print(f"Final position: [{result[0]:.1f}, {result[1]:.1f}, {result[2]:.1f}]")
    print(f"Target position: [73.0, -228.7, 119.1]")

def test_side1_sequence():
    """Test the exact transformation sequence for Side 1"""
    print("\nTesting Side 1 transformation sequence...")
    
    # Test LED 1 which we know works
    x, y = 1.49, -0.02
    m = Matrix3D()
    point = [x, y, 0]
    
    print(f"Initial: [{point[0]:.1f}, {point[1]:.1f}, {point[2]:.1f}]")
    
    # 1. Z rotation
    m.rotate_z(math.pi/10)
    point = m.apply(point)
    print(f"After Z rotation: [{point[0]:.1f}, {point[1]:.1f}, {point[2]:.1f}]")
    
    # 2. Scale
    point = [p * 5.15 for p in point]
    print(f"After scale: [{point[0]:.1f}, {point[1]:.1f}, {point[2]:.1f}]")
    
    # 3. X rotation
    m = Matrix3D()
    m.rotate_x(1.1071)
    point = m.apply(point)
    print(f"After X rotation: [{point[0]:.1f}, {point[1]:.1f}, {point[2]:.1f}]")
    
    # 4. Final position
    m = Matrix3D()
    m.translate(0, 0, 200)
    point = m.apply(point)
    print(f"Final position: [{point[0]:.1f}, {point[1]:.1f}, {point[2]:.1f}]")
    
    # Compare to target
    print(f"Target position: [73.0, -228.7, 119.1]")
    
    # Show matrix state
    print("\nFinal matrix:")
    print_matrix(m.m)

if __name__ == "__main__":
    test_led_coordinate_adjustment()
    test_led_rotation()
    test_led_translation()
    test_side1_led_sequence()
    test_side1_processing_sequence()
    test_side1_sequence() 