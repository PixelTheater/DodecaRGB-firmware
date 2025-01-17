# --- Imports ---
import math
from test_matrix3d import Matrix3D
import os
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np

# --- Constants from Processing ---
TWO_PI = 2 * math.pi
zv = TWO_PI/20  # Rotation between faces
ro = TWO_PI/5   # Rotation for pentagon points
xv = 1.1071     # angle between faces
radius = 200    # Base radius for pentagon faces
scale = 5.15    # LED position scaling factor

# Side rotation configuration from Processing
side_rotation = [
    0,  # side 0 (bottom)
    3,  # side 1
    4,  # side 2
    4,  # side 3
    4,  # side 4
    4,  # side 5
    2,  # side 6
    2,  # side 7
    2,  # side 8
    2,  # side 9
    2,  # side 10
    0   # side 11 (top)
]

class LEDPoint:
    """Match the C++ LED_Point structure"""
    def __init__(self, index: int, x: float, y: float, z: float, 
                 side: int, label_num: int):
        self.index = index
        self.x = x
        self.y = y
        self.z = z
        self.side = side
        self.label_num = label_num
    
    def __str__(self):
        return f"Point {self.index}: [{self.x:.1f}, {self.y:.1f}, {self.z:.1f}] side={self.side} label={self.label_num}"
def generate_face_points(radius: float, matrix: Matrix3D, side: int, start_index: int, rotation: int = 0):
    """Generate points for one face with given rotation (0-4)"""
    points = []
    
    # Save face position
    matrix.push_matrix()
    
    # Apply face rotation (0-4 positions)
    matrix.rotate_z(rotation * ro)  # ro = TWO_PI/5
    
    # Generate each LED position relative to face center
    for i in range(5):
        matrix.push_matrix()  # Save position for this LED
        
        # Calculate LED position (relative to face center)
        angle = i * ro
        x = radius * math.cos(angle)
        y = radius * math.sin(angle)
        z = 0
        
        transformed = matrix.apply([x, y, z])
        point = LEDPoint(start_index + i, transformed[0], transformed[1], 
                        transformed[2], side, i)
        points.append(point)
        
        matrix.pop_matrix()  # Restore face position
    
    matrix.pop_matrix()  # Restore global position
    return points

def generate_all_points():
    """Generate all points for the dodecahedron"""
    m = Matrix3D()
    points = []
    
    # Initial transform to match Processing
    m.translate(400, 400, 0)  # Center
    m.rotate_x(math.pi)       # Flip upside down
    
    # Generate points for each face
    radius = 100  # From Processing
    for side in range(10):  # 10 faces
        m.push_matrix()
        m.rotate_z(side * zv)
        face_points = generate_face_points(radius, m, side, side * 5)
        points.extend(face_points)
        m.pop_matrix()
    
    return points

def load_cpp_points():
    """Load points from points.cpp for comparison"""
    cpp_points = []
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    points_path = os.path.join(project_root, 'src', 'points.cpp')
    
    print(f"Loading reference points from: {points_path}")
    if not os.path.exists(points_path):
        raise FileNotFoundError(f"points.cpp not found at: {points_path}")
        
    with open(points_path, 'r') as f:
        contents = f.read()
        if 'LED_Point points[] = {' not in contents:
            raise ValueError(f"Could not find LED_Point array in {points_path}")
            
        in_array = False
        for line in contents.split('\n'):
            if 'LED_Point points[] = {' in line:
                in_array = True
                continue
            if in_array and line.strip() == '};':
                break
            if in_array and 'LED_Point(' in line:
                try:
                    # Parse line like: LED_Point(0, -1.43, -0.44, 268.0, 1, 0),
                    point_str = line.split('LED_Point(')[1].split(')')[0]
                    nums = point_str.split(',')
                    # Format: index, x, y, z, side, label
                    x = float(nums[1].strip())
                    y = float(nums[2].strip())
                    z = float(nums[3].strip())
                    cpp_points.append([x, y, z])
                except (IndexError, ValueError) as e:
                    print(f"Error parsing line: {line}")
                    raise ValueError(f"Failed to parse point data: {e}")
    
    if len(cpp_points) == 0:
        raise ValueError(f"No points loaded from {points_path}")
        
    print(f"Loaded {len(cpp_points)} points from points.cpp")
    return cpp_points

def compare_points(generated, cpp_points):
    """Compare generated points with C++ points"""
    print("\nComparing with points.cpp:")
    for i, (gen, cpp) in enumerate(zip(generated, cpp_points)):
        diff = [abs(g - c) for g, c in zip(gen, cpp)]
        max_diff = max(diff)
        if max_diff > 0.5:  # Allow small floating point differences
            print(f"Point {i} mismatch:")
            print(f"  Generated: [{gen[0]:.1f}, {gen[1]:.1f}, {gen[2]:.1f}]")
            print(f"  C++:       [{cpp[0]:.1f}, {cpp[1]:.1f}, {cpp[2]:.1f}]")
            print(f"  Diff:      [{diff[0]:.1f}, {diff[1]:.1f}, {diff[2]:.1f}]")

def test_single_pentagon():
    """Test generation of a single pentagon's points"""
    print("\nTesting single pentagon generation...")
    m = Matrix3D()
    m.translate(400, 400, 0)  # Center
    
    # Generate one pentagon's points
    points = generate_face_points(100, m, side=0, start_index=0)
    
    print("\nPentagon points:")
    for p in points:
        print(p)  # Use LEDPoint's __str__ method
        
    # Test point properties
    assert len(points) == 5, "Pentagon should have 5 points"
    assert points[0].index == 0, "First point should have index 0"
    assert points[0].side == 0, "Points should be on side 0"
    assert points[0].label_num == 0, "First point should have label 0"
    
    # Test first point position (should be at (500,400,0))
    assert abs(points[0].x - 500.0) < 0.1, "X coordinate wrong"
    assert abs(points[0].y - 400.0) < 0.1, "Y coordinate wrong"
    assert abs(points[0].z - 0.0) < 0.1, "Z coordinate wrong"
    
    print("Single pentagon test passed!")

def test_pentagon_orientations():
    """Test pentagon point order in different orientations"""
    print("\nTesting pentagon orientations...")
    
    # Test 1: Basic pentagon (no flip)
    m = Matrix3D()
    m.translate(400, 400, 0)
    points_no_flip = generate_face_points(100, m, side=0, start_index=0)
    print("\nPentagon without flip:")
    for p in points_no_flip:
        print(p)
    
    # Test 2: Flipped pentagon (like in dodecahedron)
    m = Matrix3D()
    m.translate(400, 400, 0)
    m.rotate_x(math.pi)
    points_flipped = generate_face_points(100, m, side=0, start_index=0)
    print("\nPentagon with flip:")
    for p in points_flipped:
        print(p)
    
    # Verify point order is mirrored
    assert abs(points_no_flip[1].y - 495.1) < 0.1, "Top point wrong before flip"
    assert abs(points_flipped[1].y - 304.9) < 0.1, "Bottom point wrong after flip"
    
    print("Pentagon orientation tests passed!")

def transform_led_point(x: float, y: float, num: int, sideNumber: int):
    """Transform LED point exactly like Processing's buildLedsFromComponentPlacementCSV()"""
    m = Matrix3D()
    
    if sideNumber == 0:  # TOP face
        # Skip initial LED space rotation for Side 0
        m.rotate_x(math.pi)  # Flip upside down
        m.rotate_z(ro)      # Global rotation
        m.rotate_z(-zv - ro*2)  # Side 0 positioning
    else:
        # Keep original sequence for other sides
        m.rotate_z(-math.pi/5)  # Initial LED space rotation
        m.rotate_x(math.pi)
        m.rotate_z(ro)
        # ... rest of transformations
    
    # Move face out to radius
    m.translate(0, 0, radius*1.34)
    
    # Additional hemisphere rotation BEFORE side rotation
    if sideNumber >= 6 and sideNumber < 11:  # lower hemisphere
        m.rotate_z(zv)
    elif sideNumber > 0:  # upper hemisphere except Side 0
        m.rotate_z(-zv)
    # No hemisphere rotation for Side 0
    
    # Apply side rotation last
    m.rotate_z(ro * side_rotation[sideNumber])
    
    result = m.apply([x, y, 0])
    # Swap X,Y and negate Z to match points.cpp
    return [result[1], result[0], -result[2]]

def test_led_positions():
    """Test key LED positions against known-good values from points.cpp"""
    print("\nTesting LED positions against points.cpp reference:")
    
    # Load reference points
    ref_points = load_reference_points()
    pcb_points = load_pcb_points('PickAndPlace_PCB_DodecaRGB_v2_2024-11-22.csv')
    
    # Test specific points we want to verify
    test_cases = [
        {'side': 0, 'led': 1},   # First LED on bottom
        {'side': 0, 'led': 52},  # Middle LED on bottom
        {'side': 11, 'led': 1},  # First LED on top
        {'side': 5, 'led': 104}, # Last LED on bottom hemisphere
        {'side': 6, 'led': 1},   # First LED on top hemisphere
    ]
    
    EPSILON = 0.1
    for case in test_cases:
        # Find reference point
        ref = next(p for p in ref_points 
                  if p['side'] == case['side'] and p['label'] == case['led'])
        
        # Find PCB point
        pcb = next(p for p in pcb_points if p['num'] == case['led'] - 1)
        
        # Transform point
        pos = transform_led_point(pcb['x'], pcb['y'], pcb['num'], case['side'])
        
        print(f"\nSide {case['side']}, LED {case['led']}:")
        print(f"  Reference: [{ref['x']:.1f}, {ref['y']:.1f}, {ref['z']:.1f}]")
        print(f"  Generated: [{pos[0]:.1f}, {pos[1]:.1f}, {pos[2]:.1f}]")
        
        # Check if within tolerance
        dx = abs(pos[0] - ref['x'])
        dy = abs(pos[1] - ref['y'])
        dz = abs(pos[2] - ref['z'])
        if dx > EPSILON or dy > EPSILON or dz > EPSILON:
            print(f"  Diff:      [{dx:.1f}, {dy:.1f}, {dz:.1f}]")
        else:
            print("  ✓ Matches reference")

def stripit(s):
    """Match Processing's string cleaning"""
    return s.strip().strip('"')  # Remove quotes and whitespace

def strip_units(s):
    """Convert string with units (like '0.09mm') to float, exactly like Processing"""
    s = stripit(s)  # First remove quotes and whitespace
    if s.endswith('mm'):
        s = s[:-2]  # Remove 'mm'
    try:
        return float(s)
    except ValueError:
        print(f"Error converting '{s}' to float")
        raise

def load_pcb_points(csv_path):
    """Load LED positions from Pick & Place CSV"""
    pcb_points = []
    script_dir = os.path.dirname(os.path.abspath(__file__))
    csv_path = os.path.join(script_dir, csv_path)
    
    print(f"Loading PCB points from: {csv_path}")
    if not os.path.exists(csv_path):
        raise FileNotFoundError(f"PCB file not found at: {csv_path}")
        
    with open(csv_path, 'r') as f:
        # Parse header
        header = next(f).strip()
        header_fields = [stripit(f) for f in header.split('\t')]
        
        # Find column indices
        designator_idx = header_fields.index('Designator')
        x_idx = header_fields.index('Mid X')
        y_idx = header_fields.index('Mid Y')
        
        # Process LED points
        for line_num, line in enumerate(f, 2):
            fields = [stripit(f) for f in line.split('\t')]
            ref = stripit(fields[designator_idx])
            
            if ref.startswith('LED'):
                try:
                    # Get raw coordinates
                    x = strip_units(fields[x_idx])
                    y = strip_units(fields[y_idx])
                    
                    # Apply offsets BEFORE scaling
                    x += 0.2
                    y -= 55.884
                    
                    # Apply scaling AFTER offsets
                    x *= scale
                    y *= scale
                    
                    num = int(ref.replace('LED','')) - 1
                    pcb_points.append({
                        'x': x, 
                        'y': y, 
                        'num': num,
                        'ref': ref
                    })
                except ValueError as e:
                    print(f"Error parsing line {line_num}: {line.strip()}")
                    raise
    
    print(f"Loaded {len(pcb_points)} LED positions from PCB")
    return pcb_points

def validate_geometry(vertices_by_side):
    """Validate the dodecahedron geometry"""
    print("\nValidating dodecahedron geometry...")
    
    # Get all vertices as points with looser tolerance for matching
    TOLERANCE = 5.0  # Increase tolerance to 5 units for matching vertices
    vertex_map = {}  # Map similar vertices to a canonical version
    
    for side, verts in vertices_by_side.items():
        for v in verts:
            # Round less aggressively
            v_rounded = (round(v[0], 0), round(v[1], 0), round(v[2], 0))
            # Find if this vertex is close to an existing one
            found = False
            for canonical in vertex_map.keys():
                dx = abs(canonical[0] - v_rounded[0])
                dy = abs(canonical[1] - v_rounded[1])
                dz = abs(canonical[2] - v_rounded[2])
                if dx < TOLERANCE and dy < TOLERANCE and dz < TOLERANCE:
                    vertex_map[canonical].add(side)
                    found = True
                    break
            if not found:
                vertex_map[v_rounded] = {side}
    
    # Print summary statistics
    print(f"\nFound {len(vertex_map)} unique vertices")
    print("\nVertex connections:")
    connection_counts = {}
    for sides in vertex_map.values():
        count = len(sides)
        connection_counts[count] = connection_counts.get(count, 0) + 1
    
    for count, num_vertices in sorted(connection_counts.items()):
        print(f"{num_vertices} vertices connect to {count} sides")
    
    # Print warnings for incorrect connections
    for v, sides in vertex_map.items():
        if len(sides) != 3:
            print(f"\nWarning: Vertex {v} connects to {len(sides)} sides: {sorted(sides)}")

def load_reference_points():
    """Load reference points from points.cpp"""
    points = []
    script_dir = os.path.dirname(os.path.abspath(__file__))
    points_path = os.path.join(script_dir, '..', 'src', 'points.cpp')
    
    print(f"Loading reference points from: {points_path}")
    with open(points_path, 'r') as f:
        for line in f:
            if line.startswith('LED_Point('):
                # Parse line like: LED_Point(0, -1.43, -0.44, 268.0, 0, 0),
                parts = line.strip().strip(',').split('(')[1].strip(')').split(',')
                try:
                    index = int(parts[0])    # 0-based index
                    x = float(parts[1])
                    y = float(parts[2])
                    z = float(parts[3])
                    label = int(parts[4])    # 0-based LED number
                    side = int(parts[5])     # 0-based side number
                    points.append({
                        'index': index,
                        'x': x, 'y': y, 'z': z,
                        'label': label,      # Keep 0-based
                        'side': side         # Keep 0-based
                    })
                except (ValueError, IndexError) as e:
                    print(f"Error parsing line: {line.strip()}")
                    print(f"Parts: {parts}")
                    raise
    
    print(f"Loaded {len(points)} reference points")
    
    # Verify first few points to ensure correct parsing
    if points:
        print("\nFirst few reference points:")
        for p in points[:3]:
            print(f"Point {p['index']}: [{p['x']:.2f}, {p['y']:.2f}, {p['z']:.2f}] label={p['label']} side={p['side']}")
    
    return points

def validate_led_positions(generated_points, reference_points):
    """Compare generated LED positions with reference points"""
    EPSILON = 0.1  # Allowable difference in coordinates
    
    print("\nValidating LED positions...")
    
    # Group reference points by side
    ref_by_side = {}
    for p in reference_points:
        if p['side'] not in ref_by_side:
            ref_by_side[p['side']] = []
        ref_by_side[p['side']].append(p)
    
    # Test points to check per side (matching points.cpp labels)
    test_indices = [1, 52, 104]  # First, middle, last LED on each side
    
    # Compare points for each side
    for side in range(12):
        print(f"\nSide {side}:")
        ref_points = ref_by_side[side]
        
        # Test specific points
        for idx in test_indices:
            ref = next(p for p in ref_points if p['label'] == idx)
            # Pass 0-based index to transform_led_point
            gen_pos = transform_led_point(ref['x'], ref['y'], idx - 1, side)
            
            # Compare coordinates
            dx = abs(gen_pos[0] - ref['x'])
            dy = abs(gen_pos[1] - ref['y'])
            dz = abs(gen_pos[2] - ref['z'])
            
            if dx > EPSILON or dy > EPSILON or dz > EPSILON:
                print(f"  LED {ref['label']}:")
                print(f"    Reference: [{ref['x']:.1f}, {ref['y']:.1f}, {ref['z']:.1f}]")
                print(f"    Generated: [{gen_pos[0]:.1f}, {gen_pos[1]:.1f}, {gen_pos[2]:.1f}]")
                print(f"    Diff:      [{dx:.1f}, {dy:.1f}, {dz:.1f}]")
            else:
                print(f"  LED {ref['label']}: ✓")

def visualize_model():
    """Draw the dodecahedron using matplotlib"""
    fig = plt.figure(figsize=(10, 10))
    ax = fig.add_subplot(111, projection='3d')
    
    # Define colors for each side
    side_colors = [
        'red',      # Side 0 (top)
        'orange',   # Side 1
        'yellow',   # Side 2
        'green',    # Side 3
        'blue',     # Side 4
        'purple',   # Side 5
        'pink',     # Side 6
        'cyan',     # Side 7
        'magenta',  # Side 8
        'brown',    # Side 9
        'gray',     # Side 10
        'black'     # Side 11 (bottom)
    ]
    
    # Load PCB LED positions
    pcb_points = load_pcb_points('PickAndPlace_PCB_DodecaRGB_v2_2024-11-22.csv')
    
    # Draw coordinate axes for reference
    ax.plot([0, 200], [0, 0], [0, 0], 'r-', label='X')
    ax.plot([0, 0], [0, 200], [0, 0], 'g-', label='Y')
    ax.plot([0, 0], [0, 0], [200], 'b-', label='Z')
    
    # Collect vertices by side for validation
    vertices_by_side = {}
    
    # Draw only Side 0 for now
    for sideNumber in range(1):  # Changed from range(12)
        # Generate pentagon vertices
        vertices = []
        angle = ro
        for i in range(5):
            x = radius * math.cos(angle * i)
            y = radius * math.sin(angle * i)
            vertices.append([x, y, 0])
        
        # Use exact same transformation sequence as LEDs
        m = Matrix3D()
        m.rotate_z(-math.pi/5)  # Initial LED space rotation
        m.rotate_x(math.pi)     # Flip upside down
        m.rotate_z(ro)          # Global rotation
        
        # Side positioning - match drawPentagon() exactly
        if sideNumber == 0:
            m.rotate_z(-zv - ro*2)
        elif sideNumber > 0 and sideNumber < 6:
            m.rotate_z(ro*sideNumber + zv - ro)
            m.rotate_x(xv)
        elif sideNumber >= 6 and sideNumber < 11:
            m.rotate_z(ro*sideNumber - zv + ro*3)
            m.rotate_x(math.pi - xv)
        else:
            m.rotate_x(math.pi)
            m.rotate_z(zv)
        
        # Move face out to radius
        m.translate(0, 0, radius*1.31)
        
        # Additional rotation based on hemisphere
        if sideNumber >= 6 and sideNumber < 11:
            m.rotate_z(zv)
        else:
            m.rotate_z(-zv)
        
        m.rotate_z(ro * side_rotation[sideNumber])
        
        # Transform and draw pentagon vertices
        transformed = []
        for v in vertices:
            t = m.apply(v)
            transformed.append(t)
        
        # Plot the pentagon edges
        xs, ys, zs = zip(*transformed)
        xs = list(xs) + [xs[0]]  # Close the pentagon
        ys = list(ys) + [ys[0]]
        zs = list(zs) + [zs[0]]
        ax.plot(xs, ys, zs, 'k-', linewidth=2, alpha=1.0, label=f'Side {sideNumber}')
        
        # Store vertices for validation
        vertices_by_side[sideNumber] = transformed
        
        # Draw LEDs using same transform function
        for led in pcb_points:
            world_pos = transform_led_point(led['x'], led['y'], led['num'], sideNumber)
            
            # Special marker for LED50 (bottom edge) for orientation
            if led['num'] == 49:  # LED50 (0-based index)
                ax.scatter(world_pos[0], world_pos[1], world_pos[2], 
                          c=side_colors[sideNumber], s=100, alpha=1.0,
                          marker='o', edgecolor='black')
            else:
                ax.scatter(world_pos[0], world_pos[1], world_pos[2], 
                          c=side_colors[sideNumber], s=30, alpha=0.7,
                          marker='o', edgecolor='black', facecolor='none')
    
    # Validate geometry
    validate_geometry(vertices_by_side)
    
    # Add validation against reference points
    reference_points = load_reference_points()
    validate_led_positions(pcb_points, reference_points)
    
    ax.set_box_aspect([1,1,1])
    ax.view_init(elev=30, azim=45)
    ax.legend()
    plt.show()

def print_matrix(m):
    """Helper to print matrix state"""
    for row in m.m:
        print(row)

def analyze_reference_points():
    """Analyze bounds and distribution of points in points.cpp"""
    points = load_reference_points()
    
    # Find bounds
    x_min = min(p['x'] for p in points)
    x_max = max(p['x'] for p in points)
    y_min = min(p['y'] for p in points)
    y_max = max(p['y'] for p in points)
    z_min = min(p['z'] for p in points)
    z_max = max(p['z'] for p in points)
    
    print("\nReference point bounds:")
    print(f"X: {x_min:.1f} to {x_max:.1f} (range: {x_max-x_min:.1f})")
    print(f"Y: {y_min:.1f} to {y_max:.1f} (range: {y_max-y_min:.1f})")
    print(f"Z: {z_min:.1f} to {z_max:.1f} (range: {z_max-z_min:.1f})")

def test_pentagon_alignment():
    """Verify pentagon vertices align with outermost LED positions"""
    m = Matrix3D()
    
    # Generate pentagon for side 0
    vertices = []
    for i in range(5):
        angle = i * ro
        x = radius * math.cos(angle)
        y = radius * math.sin(angle)
        vertices.append([x, y, 0])
    
    # Transform pentagon using same sequence as LEDs
    m.rotate_x(math.pi)
    m.rotate_z(-zv - ro*2)  # Side 0 positioning
    m.translate(0, 0, radius*1.34)
    m.rotate_z(-zv)
    m.rotate_z(ro * side_rotation[0])
    
    # Load PCB points for comparison
    pcb_points = load_pcb_points('PickAndPlace_PCB_DodecaRGB_v2_2024-11-22.csv')
    
    # Get outermost LED positions for side 0
    side0_leds = []
    for led in pcb_points:
        pos = transform_led_point(led['x'], led['y'], led['num'], 0)
        side0_leds.append(pos)
    
    print("\nTesting pentagon alignment:")
    print("Pentagon vertices:")
    for v in vertices:
        transformed = m.apply(v)
        print(f"  {transformed}")
    
    print("\nOutermost LED positions:")
    # Print 5 most distant LEDs from center
    # ... calculate and print

if __name__ == "__main__":
    print("\nRunning all tests:")
    print("=================")
    
    print("\n1. Testing LED positions...")
    test_led_positions()
    
    print("\n2. Loading reference points...")
    reference_points = load_reference_points()
    
    print("\n3. Loading PCB points...")
    pcb_points = load_pcb_points('PickAndPlace_PCB_DodecaRGB_v2_2024-11-22.csv')
    
    print("\n4. Validating geometry...")
    vertices_by_side = {}  # Will be populated during visualization
    validate_geometry(vertices_by_side)
    
    print("\n5. Validating LED positions...")
    validate_led_positions(pcb_points, reference_points)
    
    print("\n6. Generating visualization...")
    visualize_model()
    
    print("\nAll tests completed!")

