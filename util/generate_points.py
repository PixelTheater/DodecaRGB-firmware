import math
from matrix3d import Matrix3D
import os
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
import numpy as np
import matplotlib.animation as animation
from mpl_toolkits.mplot3d import proj3d

# Enable interactive backend
import matplotlib
matplotlib.use('TkAgg')  # or 'Qt5Agg' if you have Qt installed

# Constants from Processing
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
    m = Matrix3D()
    m.translate(400, 400, 0)  # Center
    
    # Generate one pentagon's points
    points = generate_face_points(100, m, side=0, start_index=0)
    
    # Test point properties
    assert len(points) == 5, "Pentagon should have 5 points"
    assert points[0].index == 0, "First point should have index 0"
    assert points[0].side == 0, "Points should be on side 0"
    assert points[0].label_num == 0, "First point should have label 0"
    
    # Test first point position (should be at (500,400,0))
    assert abs(points[0].x - 500.0) < 0.1, "X coordinate wrong"
    assert abs(points[0].y - 400.0) < 0.1, "Y coordinate wrong"
    assert abs(points[0].z - 0.0) < 0.1, "Z coordinate wrong"
    
    print("✓ Single pentagon test")

def test_pentagon_orientations():
    """Test pentagon point order in different orientations"""
    # Test 1: Basic pentagon (no flip)
    m = Matrix3D()
    m.translate(400, 400, 0)
    points_no_flip = generate_face_points(100, m, side=0, start_index=0)
    
    # Test 2: Flipped pentagon (like in dodecahedron)
    m = Matrix3D()
    m.translate(400, 400, 0)
    m.rotate_x(math.pi)
    points_flipped = generate_face_points(100, m, side=0, start_index=0)
    
    # Verify point order is mirrored
    assert abs(points_no_flip[1].y - 495.1) < 0.1, "Top point wrong before flip"
    assert abs(points_flipped[1].y - 304.9) < 0.1, "Bottom point wrong after flip"
    
    print("✓ Pentagon orientation test")

def transform_led_point(x: float, y: float, num: int, sideNumber: int):
    """Transform LED point exactly like Processing's buildLedsFromComponentPlacementCSV()"""
    m = Matrix3D()
    
    # Initial transform
    m.rotate_x(math.pi)
    
    # Side positioning from drawPentagon()
    if sideNumber == 0:  # bottom
        m.rotate_z(-zv - ro*2)
    elif sideNumber > 0 and sideNumber < 6:  # bottom half
        m.rotate_z(ro*sideNumber + zv - ro)
        m.rotate_x(xv)
    elif sideNumber >= 6 and sideNumber < 11:  # top half
        m.rotate_z(ro*sideNumber - zv + ro*3)
        m.rotate_x(math.pi - xv)
    else:  # sideNumber == 11, top
        m.rotate_x(math.pi)
        m.rotate_z(zv)
    
    # Move face out to radius
    m.translate(0, 0, radius*1.31)
    
    # Additional hemisphere rotation
    if sideNumber >= 6 and sideNumber < 11:
        m.rotate_z(zv)
    else:
        m.rotate_z(-zv)
    
    # Side rotation
    m.rotate_z(ro * side_rotation[sideNumber])
    
    # LED-specific transforms
    m.rotate_z(math.pi/10)
    
    # Final transform - negate Y and Z to match Processing's coordinate system
    result = m.apply([x, y, 0])
    return [result[0], -result[1], -result[2]]

def test_led_positions(pcb_points=None):
    """Test key LED positions against known-good values from points.cpp"""
    print("\nTesting LED positions against points.cpp reference:")
    
    # Load reference points
    ref_points = load_reference_points()
    
    # Load PCB points if not provided
    if pcb_points is None:
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
    project_root = os.path.dirname(script_dir)  # Go up one level from /util
    csv_path = os.path.join(project_root, 'data', csv_path)  # Look in /data folder
    
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
    # if points:
    #     print("\nFirst few reference points:")
    #     for p in points[:3]:
    #         print(f"Point {p['index']}: [{p['x']:.2f}, {p['y']:.2f}, {p['z']:.2f}] label={p['label']} side={p['side']}")
    
    return points

def validate_led_positions(generated_points, reference_points):
    """Validate LED positions against reference points from points.cpp"""
    EPSILON = 0.1  # Allowable difference in coordinates
    
    # Track validation statistics
    stats = {
        'arrangement_tests': 0,
        'arrangement_passed': 0,
        'arrangement_total_diff': 0.0,
        'orientation_tests': 0,
        'orientation_passed': 0,
        'orientation_total_diff': 0.0
    }
    
    # Group reference points by side
    ref_by_side = {}
    for p in reference_points:
        if p['side'] not in ref_by_side:
            ref_by_side[p['side']] = []
        ref_by_side[p['side']].append(p)
    
    # PART 1: Validate overall face arrangement
    arrangement_tests = [
        (0, 1, "Bottom face center LED"),
        (11, 1, "Top face center LED"),
        (1, 1, "Bottom hemisphere equator LED"),
        (6, 1, "Top hemisphere equator LED"),
        (3, 1, "Bottom hemisphere middle face LED"),
        (8, 1, "Top hemisphere middle face LED"),
    ]
    
    for side, led_num, _ in arrangement_tests:
        ref = next(p for p in ref_by_side[side] if p['label'] == led_num)
        gen_pos = transform_led_point(ref['x'], ref['y'], led_num - 1, side)
        
        # Compare coordinates
        dx = abs(gen_pos[0] - ref['x'])
        dy = abs(gen_pos[1] - ref['y'])
        dz = abs(gen_pos[2] - ref['z'])
        max_diff = max(dx, dy, dz)
        
        stats['arrangement_tests'] += 1
        stats['arrangement_total_diff'] += max_diff
        if max_diff <= EPSILON:
            stats['arrangement_passed'] += 1
    
    # PART 2: Validate individual face orientations
    orientation_test_points = [1, 52, 104]  # First, middle, last LED
    
    for side in range(12):
        ref_points = ref_by_side[side]
        
        for idx in orientation_test_points:
            ref = next((p for p in ref_points if p['label'] == idx), None)
            if ref is None:
                continue
                
            gen_pos = transform_led_point(ref['x'], ref['y'], idx - 1, side)
            
            # Compare coordinates
            dx = abs(gen_pos[0] - ref['x'])
            dy = abs(gen_pos[1] - ref['y'])
            dz = abs(gen_pos[2] - ref['z'])
            max_diff = max(dx, dy, dz)
            
            stats['orientation_tests'] += 1
            stats['orientation_total_diff'] += max_diff
            if max_diff <= EPSILON:
                stats['orientation_passed'] += 1
    
    # Calculate overall score (0-100)
    total_tests = stats['arrangement_tests'] + stats['orientation_tests']
    total_passed = stats['arrangement_passed'] + stats['orientation_passed']
    total_diff = stats['arrangement_total_diff'] + stats['orientation_total_diff']
    accuracy_score = 100 * (1.0 - min(1.0, total_diff / (total_tests * 10.0)))
    
    # Print summary
    print("\n=== LED Position Validation Summary ===")
    print(f"Face Arrangement:")
    print(f"  {stats['arrangement_passed']}/{stats['arrangement_tests']} tests passed " +
          f"({100 * stats['arrangement_passed'] / stats['arrangement_tests']:.1f}%)")
    print(f"  Average deviation: {stats['arrangement_total_diff'] / stats['arrangement_tests']:.3f} units")
    
    print(f"\nFace Orientations:")
    print(f"  {stats['orientation_passed']}/{stats['orientation_tests']} tests passed " +
          f"({100 * stats['orientation_passed'] / stats['orientation_tests']:.1f}%)")
    print(f"  Average deviation: {stats['orientation_total_diff'] / stats['orientation_tests']:.3f} units")
    
    print(f"\nOverall:")
    print(f"  {total_passed}/{total_tests} total tests passed " +
          f"({100 * total_passed / total_tests:.1f}%)")
    print(f"  Accuracy score: {accuracy_score:.1f}/100")
    
    return accuracy_score > 95  # Return True if accuracy is acceptable

def validate_dodecahedron_geometry(faces):
    """
    Validate that the generated dodecahedron geometry is mathematically correct.
    
    Args:
        faces: List of faces, where each face is a list of [x,y,z] vertex coordinates
    
    Returns:
        bool: True if geometry is valid
        
    Raises:
        AssertionError: If any geometric property is invalid, with details
    """
    EPSILON = 0.001  # Tolerance for floating point comparisons
    PENTAGON_ANGLE = 108 * math.pi / 180  # 108° in radians
    
    def distance(v1, v2):
        """Calculate distance between two vertices"""
        return math.sqrt(sum((a - b) ** 2 for a, b in zip(v1, v2)))
    
    def angle(v1, v2, v3):
        """Calculate angle between three vertices (v2 is center)"""
        a = [x1 - x2 for x1, x2 in zip(v1, v2)]
        b = [x1 - x2 for x1, x2 in zip(v3, v2)]
        dot = sum(x1 * x2 for x1, x2 in zip(a, b))
        mag_a = math.sqrt(sum(x * x for x in a))
        mag_b = math.sqrt(sum(x * x for x in b))
        return math.acos(dot / (mag_a * mag_b))
    
    # Find unique vertices (with tolerance)
    vertices = {}  # Map vertex to list of faces it belongs to
    for face_idx, face in enumerate(faces):
        for vertex in face:
            v = tuple(round(x/EPSILON)*EPSILON for x in vertex)  # Round to tolerance
            if v not in vertices:
                vertices[v] = set()
            vertices[v].add(face_idx)
    
    # Validate basic properties
    assert len(faces) == 12, f"Expected 12 faces, found {len(faces)}"
    assert len(vertices) == 20, f"Expected 20 vertices, found {len(vertices)}"
    
    # Validate vertex connections
    incorrect_vertices = [v for v, faces in vertices.items() if len(faces) != 3]
    assert not incorrect_vertices, f"Found {len(incorrect_vertices)} vertices not connected to exactly 3 faces"
    
    # Validate pentagon regularity
    edge_lengths = []
    angles = []
    
    for face in faces:
        assert len(face) == 5, f"Face has {len(face)} vertices, expected 5"
        
        # Check edge lengths
        for i in range(5):
            edge = distance(face[i], face[(i+1)%5])
            edge_lengths.append(edge)
        
        # Check angles
        for i in range(5):
            v1 = face[i]
            v2 = face[(i+1)%5]
            v3 = face[(i+2)%5]
            angles.append(angle(v1, v2, v3))
    
    # Validate measurements
    avg_edge = sum(edge_lengths) / len(edge_lengths)
    max_edge_diff = max(abs(e - avg_edge) for e in edge_lengths)
    assert max_edge_diff < EPSILON, f"Edge lengths not uniform, max deviation: {max_edge_diff:.3f}"
    
    avg_angle = sum(angles) / len(angles)
    max_angle_diff = max(abs(a - PENTAGON_ANGLE) for a in angles)
    assert max_angle_diff < EPSILON, f"Pentagon angles not 108°, max deviation: {max_angle_diff*180/math.pi:.1f}°"
    
    print(f"✓ Dodecahedron geometry valid: 20 vertices, 12 faces, all measurements within {EPSILON}")
    return True

def draw_dodecahedron(ax, collections=None):
    """Draw a dodecahedron with filled faces that match LED orientation."""
    m = Matrix3D()
    
    # Initial transform to match LED orientation
    m.rotate_x(math.pi)
    
    # Generate base pentagon vertices (at origin, in XY plane)
    pentagon = []
    for i in range(5):
        angle = i * ro  # ro = TWO_PI/5
        x = radius * math.cos(angle)
        y = radius * math.sin(angle)
        pentagon.append([x, y, 0])
    
    # Position faces using same angles as LED transforms
    faces = []
    colors = []  # Store a color for each face
    
    for side in range(12):
        m.push_matrix()
        
        # Match face positioning from transform_led_point()
        if side == 0:  # bottom
            m.rotate_z(-zv - ro*2)
        elif side > 0 and side < 6:  # bottom half
            m.rotate_z(ro*side + zv - ro)
            m.rotate_x(xv)
        elif side >= 6 and side < 11:  # top half
            m.rotate_z(ro*side - zv + ro*3)
            m.rotate_x(math.pi - xv)
        else:  # side == 11, top
            m.rotate_x(math.pi)
            m.rotate_z(zv)
        
        # Move face out to radius
        m.translate(0, 0, radius*1.31)
        
        # Additional hemisphere rotation
        if side >= 6 and side < 11:
            m.rotate_z(zv)
        else:
            m.rotate_z(-zv)
        
        # Transform pentagon vertices to face position
        face = [m.apply(v) for v in pentagon]
        faces.append(face)
        colors.append(f'C{side}')  # Use matplotlib's color cycle
        
        m.pop_matrix()
    
    # Create the 3D polygons with proper depth sorting
    poly = Poly3DCollection(faces, alpha=0.3)
    poly.set_facecolor(colors)
    poly.set_edgecolor('gray')
    poly.set_zorder(1)
    
    ax.add_collection3d(poly)
    
    if collections is not None:
        collections['faces'] = poly
    
    # Simple validation of geometry
    def validate_face(face):
        """Check if pentagon is regular"""
        edges = []
        for i in range(5):
            v1 = face[i]
            v2 = face[(i+1)%5]
            edge = math.sqrt(sum((a-b)**2 for a,b in zip(v1,v2)))
            edges.append(edge)
        
        avg = sum(edges)/len(edges)
        max_diff = max(abs(e-avg) for e in edges)
        return max_diff < 0.1  # Allow 0.1 unit variation
    
    # Validate faces
    valid = True
    for i, face in enumerate(faces):
        if not validate_face(face):
            print(f"Warning: Face {i} is not a regular pentagon")
            valid = False
    
    if valid:
        print("✓ All pentagons are regular")
    
    return faces  # Return faces for additional validation if needed

def visualize_model(pcb_points=None):
    """Draw the dodecahedron using matplotlib"""
    # Create figure and 3D axes with perspective projection
    fig = plt.figure(figsize=(10, 10))
    ax = fig.add_subplot(111, projection='3d')
    
    # Adjust the perspective - use equal scaling for all axes
    ax.get_proj = lambda: np.dot(Axes3D.get_proj(ax), np.diag([1, 1, 1, 1]))
    
    # Set camera position and focal length
    ax.dist = 10  # Reset to default distance
    
    # Enable mouse interaction
    ax.mouse_init()
    
    # Draw coordinate axes for reference
    ax.plot([0, 200], [0, 0], [0, 0], 'r-', label='X')
    ax.plot([0, 0], [0, 200], [0, 0], 'g-', label='Y')
    ax.plot([0, 0], [0, 0], [200], 'b-', label='Z')
    
    # Store collections for interactive access
    collections = {
        'faces': None,
        'leds': None,
        'face_colors': [f'C{i}' for i in range(12)],  # Store original colors
        'text': None  # For hover info
    }
    
    # Draw dodecahedron wireframe
    faces = draw_dodecahedron(ax, collections)
    
    # Pre-calculate LED positions by side
    led_positions_by_side = [[] for _ in range(12)]
    for side in range(12):
        for led in pcb_points:
            world_pos = transform_led_point(led['x'], led['y'], led['num'], side)
            led_positions_by_side[side].append(world_pos)
    
    # Convert to numpy arrays
    led_positions = np.vstack(led_positions_by_side)
    
    # Plot all LEDs
    collections['leds'] = ax.scatter(led_positions[:, 0], 
                                   led_positions[:, 1], 
                                   led_positions[:, 2],
                                   c='white', 
                                   edgecolor='black', 
                                   s=10, 
                                   alpha=1.0,
                                   depthshade=True,
                                   zorder=2,
                                   picker=True)  # Enable picking
    
    # Add hover text (initially hidden)
    collections['text'] = ax.text2D(0.02, 0.98, '', transform=ax.transAxes)
    
    def on_move(event):
        """Handle mouse movement"""
        if event.inaxes != ax:
            return
            
        # Get mouse coordinates and convert to 3D view coordinates
        x, y = event.xdata, event.ydata
        
        # Get the current view transformation
        proj = ax.get_proj()
        
        # Find closest face considering the current view
        closest_face = None
        min_dist = float('inf')
        
        for i, face in enumerate(faces):
            # Convert face vertices to numpy arrays
            face = np.array(face)
            
            # Calculate face center in 3D
            center = np.mean(face, axis=0)
            
            # Project center to screen coordinates using the correct transform method
            xs, ys, _ = proj3d.proj_transform(center[0], center[1], center[2], ax.get_proj())
            
            # Calculate screen-space distance
            dist = np.sqrt((xs - x)**2 + (ys - y)**2)
            
            # Check if this face is facing the camera (simple back-face culling)
            v1 = face[1] - face[0]  # Now works with numpy arrays
            v2 = face[2] - face[0]
            normal = np.cross(v1, v2)
            view_dir = center - np.array([ax.get_xbound()[0], ax.get_ybound()[0], ax.get_zbound()[0]])
            if np.dot(normal, view_dir) > 0:  # Face is visible
                if dist < min_dist:
                    min_dist = dist
                    closest_face = i
        
        if closest_face is not None and min_dist < 50:  # Add distance threshold
            # Highlight the face and its LEDs
            face_colors = collections['face_colors'].copy()  # Start with original colors
            for i in range(12):
                if i != closest_face:
                    face_colors[i] = 'gray'  # Dim non-highlighted faces
            collections['faces'].set_facecolor(face_colors)
            
            # Update LED colors
            led_colors = ['gray'] * len(led_positions)
            start_idx = closest_face * len(pcb_points)
            end_idx = start_idx + len(pcb_points)
            for i in range(start_idx, end_idx):
                led_colors[i] = 'white'
            collections['leds'].set_color(led_colors)
            
            # Show face info
            info_text = f'Side {closest_face}\nTop edge is highlighted'
            collections['text'].set_text(info_text)
            collections['text'].set_visible(True)
            
            # Highlight top edge of the pentagon
            top_edge = np.array([faces[closest_face][0], faces[closest_face][1]])
            if not hasattr(ax, '_top_edge'):
                ax._top_edge = ax.plot(top_edge[:, 0], top_edge[:, 1], top_edge[:, 2], 
                                     'yellow', linewidth=2)[0]
            else:
                ax._top_edge.set_data_3d(top_edge[:, 0], top_edge[:, 1], top_edge[:, 2])
                ax._top_edge.set_visible(True)
            
            fig.canvas.draw_idle()
        else:
            # Reset to original colors when not hovering over any face
            collections['faces'].set_facecolor(collections['face_colors'])
            collections['leds'].set_color('white')
            collections['text'].set_visible(False)
            if hasattr(ax, '_top_edge'):
                ax._top_edge.set_visible(False)
            fig.canvas.draw_idle()
    
    # Connect event handlers
    fig.canvas.mpl_connect('motion_notify_event', on_move)
    
    # Set equal aspect ratio for all axes
    ax.set_box_aspect([1,1,1])
    
    # Set initial view angle for isometric-like view
    ax.view_init(elev=35, azim=45)
    
    # Set axis limits to center the model
    max_range = np.array([
        led_positions[:,0].max() - led_positions[:,0].min(),
        led_positions[:,1].max() - led_positions[:,1].min(),
        led_positions[:,2].max() - led_positions[:,2].min()
    ]).max() / 2.0
    
    mid_x = (led_positions[:,0].max() + led_positions[:,0].min()) * 0.5
    mid_y = (led_positions[:,1].max() + led_positions[:,1].min()) * 0.5
    mid_z = (led_positions[:,2].max() + led_positions[:,2].min()) * 0.5
    
    ax.set_xlim(mid_x - max_range, mid_x + max_range)
    ax.set_ylim(mid_y - max_range, mid_y + max_range)
    ax.set_zlim(mid_z - max_range, mid_z + max_range)
    
    # Reduce number of ticks and adjust their appearance
    ax.set_xticks(np.linspace(ax.get_xlim()[0], ax.get_xlim()[1], 5))
    ax.set_yticks(np.linspace(ax.get_ylim()[0], ax.get_ylim()[1], 5))
    ax.set_zticks(np.linspace(ax.get_zlim()[0], ax.get_zlim()[1], 5))
    
    # Make axis labels and ticks smaller for cleaner look
    ax.tick_params(labelsize=8)
    
    ax.legend(fontsize=8)
    
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

if __name__ == "__main__":
    # Run all tests
    test_single_pentagon()
    test_pentagon_orientations()
    
    # Load PCB points once
    pcb_points = load_pcb_points('PickAndPlace_PCB_DodecaRGB_v2_2024-11-22.csv')
    
    # Use loaded points for both functions
    test_led_positions(pcb_points)
    visualize_model(pcb_points)