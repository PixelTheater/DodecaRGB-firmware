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
    // ... model-specific data ...
};

}} // namespace PixelTheater::Models
```

## Model Organization

Models follow a specific folder structure:

```
src/models/
└── DodecaRGBv2/              # Model directory
    ├── model.yaml            # Model definition
    ├── model.h              # Generated header file
    ├── pcb/                 # PCB-related files
    │   ├── DodecaRGB.pos    # Pick-and-place file
    │   └── board.json       # Board definition
    └── README.md            # Model documentation
```

## Core Components

A Model consists of three main collections that work together to represent the physical LED arrangement:

1. **LED Array** (`leds`): 
   - Direct access to LED color data
   - Compatible with FastLED
   - Zero-based continuous indexing
   - Bounds-checked access

2. **Point Geometry** (`points`):
   - 3D coordinates for each LED
   - Face assignments
   - Neighbor relationships
   - Distance calculations

3. **Face Hierarchy** (`faces`):
   - Physical surface definitions
   - LED groupings
   - Geometric transformations
   - Local coordinate systems

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
      edge: [0-19, 20-39, 40-59, 60-79, 80-99]
      center: [100-103]

faces:
  - id: 0
    type: pentagon
    rotation: 0
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
python util/generate_model.py -d src/models/DodecaRGBv2
```

This creates `model.h` with:
- LED count and face count as template parameters
- Face type definitions with vertices
- Point coordinates and face assignments
- Neighbor relationships
- Model metadata

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
   - Physical dimensions
   - LED arrangement
   - Coordinate system
   - Special considerations
   - Assembly instructions

See the DodecaRGBv2 model for a complete example.
