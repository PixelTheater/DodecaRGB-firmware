import math
import os


if __name__ == "__main__":
    print("This is a utility module for DodecaRGB. It is not meant to be run directly.") 
    exit(1)

from util.matrix3d import Matrix3D

# Constants shared between Python and C++
MAX_LED_NEIGHBORS = 7

# Constants from Processing
TWO_PI = 2 * math.pi
zv = TWO_PI/20  # Rotation between faces
ro = TWO_PI/5   # Rotation for pentagon points
xv = 1.1071     # angle between faces
radius = 200    # Base radius for pentagon faces
scale = 5.15    # LED position scaling factor

# Side rotation configuration - each number represents the number of times the pentagon PCB
# is rotated relative to the starting position, in 72 degree increments.
# NOTE: This array is now deprecated - use the rotation parameter in transform functions instead
side_rotation = [
    0,  # side 0 (bottom)
    0,  # side 1
    0,  # side 2
    0,  # side 3
    0,  # side 4
    0,  # side 5
    0,  # side 6
    0,  # side 7
    0,  # side 8
    0,  # side 9
    0,  # side 10
    0   # side 11 (top)
]

# Colors for visualization
FACE_COLORS = [
    (1.0, 0.0, 0.0),  # Red
    (0.0, 0.8, 0.0),  # Green
    (0.0, 0.0, 1.0),  # Blue
    (1.0, 1.0, 0.0),  # Yellow
    (1.0, 0.0, 1.0),  # Magenta
    (0.0, 1.0, 1.0),  # Cyan
    (1.0, 0.5, 0.0),  # Orange
    (0.5, 0.0, 1.0),  # Purple
    (0.0, 0.5, 0.5),  # Teal
    (1.0, 0.0, 0.5),  # Pink
    (0.5, 1.0, 0.0),  # Lime
    (0.0, 0.5, 1.0),  # Sky Blue
]

def transform_led_point(x: float, y: float, num: int, sideNumber: int, rotation: int = 0):
    """Transform LED point exactly like Processing's buildLedsFromComponentPlacementCSV()
    
    Args:
        x, y: LED coordinates from PCB file
        num: LED number (0-based)
        sideNumber: Face number (0-11) - should be geometric ID for proper positioning
        rotation: Face rotation in 72-degree increments (0-4) from YAML config
    """
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
    
    # Side rotation - now uses the rotation parameter from YAML config
    m.rotate_z(ro * rotation)
    
    # LED-specific transforms
    m.rotate_z(-math.pi/10)
    
    # Final transform - negate Y and Z to match Processing's coordinate system
    result = m.apply([x, y, 0])
    return [result[0], -result[1], -result[2]]

def strip_units(value_str):
    """Strip units (mm or mil) from coordinate strings and convert to mm"""
    value_str = value_str.strip()
    if value_str.endswith('mil'):
        # Convert mil to mm (1000 mil = 25.4 mm)
        return float(value_str.replace('mil', '')) * 25.4 / 1000.0
    elif value_str.endswith('mm'):
        return float(value_str.replace('mm', ''))
    else:
        # Try to parse as number (assume mm if no units)
        return float(value_str)

def stripit(s):
    """Strip whitespace and quotes"""
    return s.strip().strip('"')

def load_pcb_points(filename):
    """Load LED positions from PCB pick and place file"""
    pcb_points = []
    
    print(f"Loading PCB points from: {filename}")
    if not os.path.exists(filename):
        raise FileNotFoundError(f"PCB file not found at: {filename}")
    
    # Detect encoding by reading first few bytes
    encoding = 'utf-8'
    units_detected = 'mm'
    
    with open(filename, 'rb') as f:
        first_bytes = f.read(4)
        if first_bytes.startswith(b'\xff\xfe'):
            encoding = 'utf-16-le'
            print(f"  Detected UTF-16 Little Endian encoding")
        elif first_bytes.startswith(b'\xfe\xff'):
            encoding = 'utf-16-be'
            print(f"  Detected UTF-16 Big Endian encoding")
        else:
            print(f"  Using UTF-8 encoding")
    
    # Read and detect units
    with open(filename, 'r', encoding=encoding) as f:
        content = f.read()
        if 'mil' in content.lower():
            units_detected = 'mil'
            print(f"  Detected mil units - will convert to mm (1000mil = 25.4mm)")
        else:
            print(f"  Detected mm units")
    
    # Now parse the file properly
    with open(filename, 'r', encoding=encoding) as f:
        # Parse header
        header = next(f).strip()
        # Remove BOM character if present (common in UTF-16 files)
        if header.startswith('\ufeff'):
            header = header[1:]
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
                    x += 0
                    y -= 0
                    
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
