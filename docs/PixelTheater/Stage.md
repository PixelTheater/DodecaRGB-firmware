---
category: Developer Reference
version: 2.8.3
---

# Stage Reference

## Core Classes

### Model

A geometric model with LED mapping. Models can be generated from hardware specifications (YAML + pick-and-place files) using the utility scripts.

### Model Classes

Models can have different face types and topologies. PixelTheater provides a consistent API for accessing the geometry of any model type, and lighting up LEDs, regions, and faces.

Model `Geometry` classes:

- `Model` - The main model class
- `Face` - A single face of the model

LED segment `Region` classes that return a list of LEDs:

- `Edge` - An edge of a face
- `Ring` - A ring of LEDs on a face

You can also define your own custom regions by extending the `Region` base class.

### Geometry Access Patterns

The model provides clear relationships between geometric elements and LEDs:

### Model API: A brief list of geometry access methods:

Model Regions:

- `model.faces` - List of all faces
- `model.edges` - List of all edges
- `model.rings` - List of all rings

Face Regions: given `auto face = model.faces[0];`

- `face.edges` - List of all edges on the face
- `face.rings` - List of all rings on the face
- `face.center` - Center point of the face

Region Components: given `auto edge = face.edges[0];`

- `edge.leds` - List of all LEDs on the edge
- `edge.center` - Center point of the edge
- `edge.length` - Length of the edge
- `edge.width` - Width of the edge
- `edge.normal` - Normal vector of the edge
- `edge.middle` - Middle point of the edge
- `edge.start` - Start point of the edge
- `edge.end` - End point of the edge

The `leds` property is a list of all LEDs in the model. It can be accessed as an array or iterated over, or inspected to find the properties of the region. As it contains CRGB objects, it can be manipulated like a FastLED array.

### Usage Examples

```cpp
// Direct LED access
model.leds[42].color = CRGB::Blue;

// Face and region access
auto& face = model.faces[3];
face.rings[2].fill(CRGB::Red);        // Fill third ring
face.edges[0].fill(CRGB::Green);      // Fill first edge
face.center.fill(CRGB::White);        // Fill center LED
face.edges[0].middle().fill(CRGB::Yellow); // Fill center of first edge

// Point to LED relationships
float x = model.leds[42].point().x();  // get x coordinate of LED 42
Point p = face.center();              // Get face center point
auto nearby = model.findNearby(p, 1.0f);
nearby.fill(CRGB::Blue);

// Iteration with geometry
for(auto& face : model.faces) {
    // Color based on height
    float y = face.center().y();
    uint8_t hue = map(y, -50, 50, 0, 255);
    
    // Access rings by index
    for(int r = 0; r < 5; r++) {
        face.rings[r]
            .fill(CHSV(hue, 255, 255))
            .fadeToBlackBy(r * 32);  // Outer rings darker
    }
}

// LED to Point relationships
for(auto& led : face.leds) {
    float height = led.point().y();
    led.color = CHSV(height * 8, 255, 255);
}
```

All array access and methods are bounds-checked but will not crash on invalid inputs:

- Out of range indices return last valid element
- Invalid geometric queries return zero/empty results
- Color operations on empty spans are safely ignored

### Setup and Configuration in main.cpp

```cpp
// In main.cpp
#include "models/DodecaRGBv2r0.h"

CRGB leds[Model::NUM_LEDS];  

void setup() {
    FastLED.addLeds<WS2812B, 19, GRB>(leds, 0, Model::NUM_LEDS/2);
    FastLED.addLeds<WS2812B, 18, GRB>(leds + Model::NUM_LEDS/2, Model::NUM_LEDS/2);
    
    stage.addLeds(leds);
    stage.brightness(128);
    stage.leds.fill(CRGB::Red);
    stage.update();
}
```



### Error Handling Strategy

Since we can't use exceptions on the microcontroller:

1. Array access is bounds-checked but returns safe values:
   - Out of range face index returns last valid face
   - Out of range LED index returns last LED
   - Invalid ring/edge numbers return empty spans

2. Methods return sentinel values:
   - Invalid geometric queries return zero vectors
   - Size queries return 0 for invalid inputs
   - Color operations silently skip invalid LEDs

3. Debug output warnings:
    - when validations fail, warnings are printed to the console
    - Example: "Warning: Face index %d out of range"

This strategy prevents crashes while making issues visible during testing.


### Model Definition

Models need minimal YAML configuration:

```yaml
# models/icosidodecahedron-v1.yaml
name: IcosidodecahedronV1
version: "1.0.0"
description: "Mixed pentagon/triangle LED polyhedron"

# Physical properties
shape: Icosidodecahedron
edge_length_mm: 50.0
led_diameter_mm: 5.0
led_spacing_mm: 8.0  # Average spacing between LEDs

# Face definitions
face_types:
  pentagon:
    leds_per_face: 104
    pnp_file: pentagon-face.csv
  triangle:
    leds_per_face: 45
    pnp_file: triangle-face.csv

# Assembly sequence - defines how PCBs connect in series
# - face: PCB number (0-11) for assembly and wiring order
# - type: PCB shape type
# - rotation: 0-4 for pentagon (72° increments clockwise)
face_map:
  - face: 0          # First PCB in series
    type: pentagon
    rotation: 1      # 72° clockwise
  
  - face: 1          # Second PCB
    type: pentagon   
    rotation: 2      # 144° clockwise
  
  - face: 2          # Third PCB
    type: pentagon
    rotation: 0      # no rotation
  
  # ... etc
```

### Region Generation

The python generator calculates regions from LED positions:

```python
# In util/generate_model.py

def calculate_regions(points, face_type):
    """Calculate LED regions from physical layout"""
    

def process_pick_and_place(pnp_file, face_type):
    """Process pick-and-place data for a face type"""
    

# Updates the led positions and regions for each face type
# which is written to the generated model file
```

The generator:

1. Reads minimal YAML config defining face types and PCB assembly sequence
2. Builds a 3d virtaul model of the desired shape
4. Processes pick-and-place files for each face type
3. Applies face rotations from face_map and projects each side into place
5. Calculates 3D coordinates and nearest LEDs for each physical position
6. Creates region mappings using face type definitions
7. Outputs C++ implementation files

### Point Generation

The points are generated from pick-and-place data:

```bash
# Generate model implementation
python util/generate_model.py models/icosidodecahedron-v1/model.yaml --output src/models/

# This creates:
# - src/models/IcosidodecahedronV1.h    # Model interface
# - src/models/IcosidodecahedronV1.cpp  # Generated points and regions
```

### Generated Model Files

The generator creates a single header with just the model data:

```cpp
// In src/models/DodecaRGBv2.h
namespace Model {
    // Core model properties
    static constexpr uint16_t NUM_LEDS = 1248;
    static constexpr uint8_t NUM_FACES = 12;
    static constexpr uint8_t LEDS_PER_FACE = 104;
    
    // Model metadata
    namespace Config {
        static constexpr char NAME[] = "DodecaRGBv2";
        static constexpr char VERSION[] = "2.0.0";
        static constexpr float EDGE_LENGTH_MM = 60.0f;
    }

    // Pre-calculated geometric data
    namespace Data {
        // All LED positions in 3D space with nearest neighbors
        static const Point POINTS[NUM_LEDS] = {
            // Face 0
            LED_Point(0, -1.427f, -0.442f, 262.000f, 0,  // LED 0: x,y,z, face
                {{.led_number = 1, .distance = 25.256f},     // Nearest neighbors
                 {.led_number = 3, .distance = 25.653f},
                 {.led_number = 2, .distance = 25.660f},
                 {.led_number = 5, .distance = 26.237f},
                 {.led_number = 4, .distance = 26.780f}}),
            // ... etc for all LEDs
        };

        // Named region definitions
        namespace Regions {
            const Region CENTER_0{"center_0", {0, 1, 2, 3}};
            const Region RING_0_0{"ring_0_0", {8, 9, 10, 11, 12, 13}};
            const Region EDGE_0_0{"edge_0_0", {80, 81, 82, 83, 84}};
            // ... etc
        }

        // Face geometry and region assignments
        static const Face FACES[NUM_FACES] = {
            {
                .center = Point{0.0f, 1.0f, 0.0f},
                .normal = Vector3d{0.0f, 1.0f, 0.0f},
                .rotation = 1,
                .regions = {
                    .center = Regions::CENTER_0,
                    .rings = {Regions::RING_0_0, Regions::RING_0_1},
                    .edges = {Regions::EDGE_0_0, Regions::EDGE_0_1}
                }
            },
            // ... etc for all faces
        };
    }
}
```

The Stage and Geometry classes then use this data to implement the model interface. This keeps the generated files focused purely on the model-specific data, while the API implementation lives in the core library files.
