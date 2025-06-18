# Model System

The Model system in PixelTheater represents physical LED arrangements and their geometric relationships. Models are defined through YAML configuration and generated into C++ header files that provide type-safe access to LED data and geometric information.

## Namespace Organization

Models are defined in the `PixelTheater::Models` namespace to avoid naming conflicts. Each model is defined in its own header file:

```cpp
// In models/DodecaRGBv2/model.h
namespace PixelTheater {
namespace Models {

struct DodecaRGBv2 : public ModelDefinition<1248, 12> {
    static constexpr const char* NAME = "DodecaRGBv2";
    static constexpr const char* VERSION = "2.0.0";
    static constexpr const char* DESCRIPTION = "12-sided RGB LED dodecahedron";
    static constexpr const char* MODEL_TYPE = "dodecahedron";
    static constexpr float SPHERE_RADIUS = 168.250f; // Example value
    // ... model-specific data ...
};

}} // namespace PixelTheater::Models
```

## Model Organization

Models follow a specific folder structure:

```
src/models/
└── DodecaRGBv2/             # Model directory
    ├── model.yaml           # Model definition
    ├── model.h              # Generated header file
    ├── pcb/                 # PCB-related files
    │   ├── DodecaRGB.csv    # Pick-and-place file
    └── README.md            # Model documentation
```

## Core Components

A Model consists of four main collections that work together to represent the physical LED arrangement:

1. **LED Array** (`leds`): 
   - Direct access to LED color data
   - Compatible with FastLED
   - Zero-based continuous indexing
   - Bounds-checked access

2. **Point Geometry** (`points`):
   - 3D coordinates for each LED
   - Face assignments
   - Access to pre-calculated neighbor relationships (`getNeighbors()`)
   - Distance calculations

3. **Face Hierarchy** (`faces`):
   - Physical surface definitions
   - LED groupings
   - Geometric transformations
   - Local coordinate systems

4. **Edge Connectivity** (`edges`):
   - Face adjacency information
   - Edge-to-edge relationships
   - Boundary definitions
   - Topology validation

5. **LED Groups** (`groups`):
   - Named collections of LEDs within faces
   - Logical groupings (e.g., "center", "ring0", "edge")
   - Simplified access for common patterns
   - Face-type specific organization

## Data Organization

The Model maintains parallel arrays that represent each physical LED:

```cpp
// Internal storage (simplified)
namespace PixelTheater {
template<typename ModelDef>
class Model {
private:
    CRGB* _leds;                                        // LED colors (FastLED)
    std::array<Point, ModelDef::LED_COUNT> _points;     // 3D geometry
    std::array<Face, ModelDef::FACE_COUNT> _faces;      // Surface hierarchy
};
}
```

These arrays are always synchronized:
- Same indexing scheme (leds[i] and points[i] refer to same LED)
- Consistent face assignments
- Maintained automatically by the Model class

### Neighbor Data

In addition to coordinates and face assignments, each `Point` object stores a list of its nearest neighbors, pre-calculated during model generation. This allows for efficient traversal of the model's topology without expensive runtime distance calculations.

```cpp
// Access a point
const Point& p = model.points[42];

// Get its neighbors (returns a const reference to an array)
const auto& neighbors = p.getNeighbors();

// Iterate through neighbors
for (const auto& neighbor : neighbors) {
    // Check if the neighbor entry is valid (using sentinel check from point.cpp)
    if (neighbor.id == 0xFFFF || neighbor.distance <= 0.0f) continue; 

    // Access neighbor info
    uint16_t neighbor_id = neighbor.id;
    float distance_to_neighbor = neighbor.distance;

    // Access the neighbor point itself
    if (neighbor_id < model.points.size()) { // Bounds check
        const Point& neighbor_point = model.points[neighbor_id];
        // ... use neighbor_point.x(), neighbor_point.y(), etc. ...
    }
}
```

The neighbor list is ordered by distance, with the closest neighbor first. The size of the neighbor list is fixed at compile time (`PixelTheater::Limits::MAX_NEIGHBORS`).

## Coordinate Systems

Models support multiple coordinate systems:

1. **World Space**:
   - Global 3D coordinates
   - Used for distance calculations
   - Origin at model center

2. **Face Space**:
   - Local to each face
   - Used for surface effects
   - Origin at face center

3. **LED Indices**:
   - Zero-based continuous array
   - Used for direct LED access
   - Maps to physical wiring

Example coordinate relationships:
```cpp
// World space position of LED
Point& p = model.points[42];
float x = p.x();  // Global X coordinate
float y = p.y();  // Global Y coordinate
float z = p.z();  // Global Z coordinate

// Face space mapping
Face& f = model.faces[p.face_id()];
size_t local_idx = f.local_index(42);  // LED index within face

// Direct LED access
CRGB& led = model.leds[42];  // Color data for same LED
```

## Edge Access

Models provide face-centric access to edge connectivity for topology-aware animations:

```cpp
// Get a face proxy for face-centric operations
auto face_proxy = model().face(face_id);

// Get number of edges for this face
uint8_t edges_per_face = face_proxy.edge_count();

// Get connected face for a specific edge of this face
int8_t adjacent_face = face_proxy.face_at_edge(edge_index);  // Returns -1 if no connection

// Iterate through all edges of this face
for (int edge_idx = 0; edge_idx < edges_per_face; edge_idx++) {
    int8_t adjacent_face = face_proxy.face_at_edge(edge_idx);
    if (adjacent_face >= 0) {
        // Edge edge_idx of this face connects to adjacent_face
        // Use this for adjacency-based effects like edge coloring
    }
}

// Alternative: iterate using the edges() method
auto face_edges = face_proxy.edges();
for (const auto& edge : face_edges) {
    if (edge.has_connection()) {
        int8_t adjacent_face = edge.connected_face_id;
        // Process edge connection...
    }
}
```

## LED Groups Access

Models provide named access to logical groups of LEDs within faces:

```cpp
// Get a face proxy for face-centric operations  
auto face_proxy = model().face(face_id);

// Access a LED group by name for this face
auto face_center = face_proxy.group("center");
face_center[0] = CRGB::Red;  // Light first LED in the center group

// Check if group exists and has LEDs
if (face_center.size() > 0) {
    face_center[0] = CRGB::Blue;  // Access specific LED in group
}

// Iterate through all LEDs in a group
for (auto& led : face_center) {
    led = CRGB::White;  // Light all center LEDs for this face
}

// Get all available group names for this face
auto groups = face_proxy.groups();
for (const char* group_name : groups) {
    auto group = face_proxy.group(group_name);
    // Process each group...
}
```

Common LED group names (model-dependent):
- `"center"`: Center LEDs of a face
- `"ring0"`, `"ring1"`, etc.: Concentric rings of LEDs
- `"edge0"`, `"edge1"`, etc.: LEDs along specific edges
- `"corner"`: Corner/vertex LEDs

## Model Definition

Models are defined in YAML with explicit geometric relationships:

```yaml
model:
  name: "DodecaRGBv2"
  version: "2.0.0"
  description: "12-sided RGB LED dodecahedron"
  author: "PixelTheater Team"

geometry:
  shape: "dodecahedron"
  edge_length_mm: 130.0
  
face_types:
  pentagon:
    num_leds: 104
    num_sides: 5
    groups:
      edge: [1,2,3,4,5,6,7,8]
      center: [12]
      smiley: [2,3,4,62,43,13,26,36,46,24,52,63]

faces:
  - id: 0
    type: triangle
    rotation: 0
  - id: 1
    type: triangle
    rotation: 1
    remap_to: 3
  - id: 2
    type: triangle
    rotation: 2
  # ... additional faces ...
```

The YAML definition is processed by the model generator to create a C++ header file that provides:
- Type-safe access to LED data
- Compile-time constants
- Geometric relationships
- Neighbor calculations

## Model Generation

Models are generated using the `generate_model.py` utility which creates a header file containing the complete model definition:

```bash
# Generate model from a model directory (recommended)
python util/generate_model.py -d src/models/DodecaRGBv2

# Or specify individual files
python util/generate_model.py -m src/models/DodecaRGBv2/model.yaml -i src/models/DodecaRGBv2/pcb/DodecaRGB.csv
```

The model generator:
1. Reads the YAML model definition file that specifies face types, face instances, and model metadata
2. Loads PCB pick-and-place data to get LED positions
3. Transforms LED positions based on face positions and rotations
4. Calculates neighbor relationships between LEDs
5. Calculates the model's bounding sphere radius
6. Generates a C++ header file with the complete model definition

This creates `model.h` with:
- LED count and face count as template parameters
- Compile-time constants like `SPHERE_RADIUS`
- Face type definitions with vertices
- Point coordinates and face assignments
- Neighbor relationships
- Model metadata

### Command Line Options

```
Options:
  -m, --model MODEL       Path to model YAML definition file
  -d, --model-dir DIR     Path to model directory containing model.yaml and pcb/*.pos
  -o, --output FILE       Output file (default: model.h in model directory)
  -f, --format FORMAT     Output format: cpp or json (default: cpp)
  -i, --input FILE        Input PCB pick and place file (overrides YAML definition)
  -y, --yes               Automatically overwrite existing files without confirmation
```

## Best Practices

1. **Model Organization**:
   - Keep model definitions in `src/models/`
   - One directory per model with model.h as the main header
   - Include PCB data and documentation
   - Never manually edit generated model.h files

2. **Coordinate Systems**:
   - Use world space for global effects
   - Use face space for surface effects
   - Use LED indices for direct access

3. **Type Safety**:
   - Use the Models namespace
   - Let the compiler check LED counts
   - Use provided bounds-checking accessors

4. **Documentation**:
   - Document coordinate conventions
   - Explain face numbering
   - Note any special LED arrangements
   - Keep a README.md in each model directory

## Creating New Models

To create a new model:

1. Create a new directory in `src/models/`
2. Create a `model.yaml` file defining your model's properties
3. Add the PCB pick-and-place data (CSV) to the `pcb/` subdirectory
4. Run the model generator:
   ```bash
   python util/generate_model.py -d src/models/YourModel
   ```
5. Create a README.md documenting:
   - Physical dimensions (including the generated SPHERE_RADIUS)
   - LED arrangement
   - Coordinate system
   - Special considerations
   - Assembly instructions

See the DodecaRGBv2 model for a complete example.

## Face Configuration

### Face Rotation

Each face in the model can be individually rotated to match its physical orientation during assembly. Face rotation is specified using the `rotation` field in the YAML configuration:

```yaml
faces:
  - id: 0
    type: pentagon
    rotation: 2     # 144° clockwise rotation (2 × 72°)
  - id: 1  
    type: pentagon
    rotation: 0     # No rotation (0°)
  - id: 2
    type: pentagon
    rotation: 4     # 288° clockwise rotation (4 × 72°)
```

**Rotation Values:**
- `0`: No rotation (0°)
- `1`: 72° clockwise
- `2`: 144° clockwise  
- `3`: 216° clockwise
- `4`: 288° clockwise

The rotation affects both LED positioning and vertex geometry, ensuring that the generated 3D coordinates match the physical orientation of each PCB face.

### Face Remapping

Face remapping allows models to separate **logical face access** (used in scene code) from **physical wiring order** (determined by hardware assembly). This enables consistent scene behavior regardless of how the physical device was wired.

### Concepts

1. **Logical Face ID**: The face ID used in scene code (e.g., `model().face(0)`)
2. **Geometric Position**: The physical 3D position/orientation where a face is located on the model  
3. **Physical Wiring**: The order in which LEDs were connected during hardware assembly

### How Remapping Works

When `remap_to` is specified in the YAML configuration:

```yaml
faces:
  - id: 0          # Logical face ID (used in scenes)
    type: triangle
    rotation: 0
    remap_to: 2     # This face is positioned at geometric location 2
  - id: 1
    type: triangle  
    rotation: 0
    remap_to: 3     # This face is positioned at geometric location 3
  - id: 2
    type: triangle
    rotation: 0  
    remap_to: 0     # This face is positioned at geometric location 0
  - id: 3
    type: triangle
    rotation: 0
    remap_to: 1     # This face is positioned at geometric location 1
```

The system creates a mapping where:
- Scene code: `model().face(0)` → accesses face at geometric position 0 → logical face 2 → LEDs 6-8
- Scene code: `model().face(1)` → accesses face at geometric position 1 → logical face 3 → LEDs 9-11  
- Scene code: `model().face(2)` → accesses face at geometric position 2 → logical face 0 → LEDs 0-2
- Scene code: `model().face(3)` → accesses face at geometric position 3 → logical face 1 → LEDs 3-5

### Key Principles

1. **Wiring Stays Consistent**: LEDs are always wired in logical face order (face 0 → LEDs 0-2, face 1 → LEDs 3-5, etc.)
2. **Scene Access is Geographic**: `model().face(X)` accesses the face at geometric position X
3. **Remapping is Transparent**: Scene code doesn't know about remapping - it just works
4. **3D Positioning Follows Geometry**: LED coordinates are positioned based on geometric location

### Without Remapping

```cpp
// No remap_to specified - logical ID matches geometric position
model().face(0)   // → logical face 0 → LEDs 0-2 → geometric position 0
model().face(1)   // → logical face 1 → LEDs 3-5 → geometric position 1
```

Physical LED access and geometric position are identical.

### With Remapping  

```cpp
// With remap_to specified - scene accesses by geometric position
model().face(0)   // → geometric position 0 → logical face 2 → LEDs 6-8
model().face(1)   // → geometric position 1 → logical face 3 → LEDs 9-11
model().face(2)   // → geometric position 2 → logical face 0 → LEDs 0-2
model().face(3)   // → geometric position 3 → logical face 1 → LEDs 3-5
```

Scene code accesses faces by their geometric position, but the LED wiring follows logical face order.

### Use Cases

**Problem**: Your dodecahedron was assembled with faces wired in a different order than the geometric layout.

**Solution**: Use remapping to maintain consistent scene behavior:
```yaml
faces:
  - id: 0           # First face wired (LEDs 0-103)
    type: pentagon
    remap_to: 7     # But this face is at geometric position 7
  - id: 1           # Second face wired (LEDs 104-207)  
    type: pentagon
    remap_to: 2     # But this face is at geometric position 2
  # ... etc
```

Now scene code works correctly:
```cpp
// Scene code accesses faces by geometric position
model().face(0)  // Bottom face (geometric position 0)
model().face(7)  // Top face (geometric position 7)

// But LEDs are accessed in wiring order
// face(0) → logical face 7 → LEDs 728-831
// face(7) → logical face 0 → LEDs 0-103
```

### Implementation Details

The model system handles remapping through:

1. **Data Generation**: Positions LEDs in 3D space based on geometric location (`remap_to`)
2. **Face Access**: `model().face(X)` maps X to the logical face positioned at geometric location X
3. **LED Assignment**: LEDs remain assigned to logical faces for consistent wiring
4. **Coordinate Systems**: 3D coordinates reflect geometric positioning

### Testing Remapping

To verify remapping works correctly:

```cpp
// Test: Setting color through geometric access should light correct physical LEDs
model().face(0).leds()[0] = CRGB::Red;  // Geometric position 0

// With remapping: face 0 → logical face 2 → LED 6
// Without remapping: face 0 → logical face 0 → LED 0
```

The physical LED that lights up will change based on remapping configuration.

### Combining Rotation and Remapping

Rotation and remapping can be used together for complete control over face positioning and orientation:

```yaml
faces:
  - id: 0          # First PCB in wiring order
    type: pentagon
    rotation: 2     # Rotate 144° to match physical orientation
    remap_to: 5     # Position at geometric location 5
  - id: 1          # Second PCB in wiring order  
    type: pentagon
    rotation: 1     # Rotate 72° to match physical orientation
    remap_to: 0     # Position at geometric location 0 (bottom)
```

**Processing Order:**
1. **Rotation**: Applied first to orient the face correctly (affects LED positions and vertices)
2. **Remapping**: Applied second to position the face in 3D space (geometric location)
3. **Result**: Face appears at the correct 3D location with the correct orientation

This allows complete flexibility in accommodating any physical assembly configuration while maintaining clean, predictable scene code behavior.
