# Model Initialization and Use

In PixelTheater, a **Model** represents a physical shape made from PCBs, along with all of the geometry data and LED points and region mappings. Models are generated from YAML configuration files and hardware specifications (EDA pick-and-place files) using the utility scripts. The result is a compete set of 3d point and geometric data in a C++ header file that can be used to animate the model. This workflow allows many model shapes and sizes to be designed and remixed while maintaining support for [Scene](Scene.md) animations.

## Core Relationships

The `Model` contains collections of `Face` objects, `CRGB` colors, and `Point` coordinates.

Each `Face` represents a physical surface. There can be different types of faces on the same model, so every model defines one or more `FaceType` objects. 

for each `FaceType`, users can optionally define mutiple named `LedGroup` arrays, which are simple lists of LEDs numbers that define patterns or groups of LEDs on a face for animations. These will be avilable on each face instance.

## Understanding LEDs and Points

The model represents physical LEDs in two ways:
1. As RGB colors that can be manipulated (leds)
2. As 3D coordinates with relationships (points)

Consider a face that has 20 LEDs, on a model with 8 faces. Indexing from the model is from 0..159. But each face has its own indexing from 0..19. Therefore:

```cpp
model.leds[3] == model.faces[0].leds[3]
model.leds[47] == model.faces[2].leds[7]
model.leds[159] == model.faces[7].leds[19]
```

### Storage and Indexing

The Model maintains two parallel arrays that represent each physical LED in two ways:

```cpp
std::array<CRGB, ModelDef::LED_COUNT> _leds_data;    // LED colors (FastLED compatible)
std::array<Point, ModelDef::LED_COUNT> _points;       // 3D coordinates and relationships
```

These arrays are always synchronized:
- Same size (defined by ModelDef::LED_COUNT)
- Same indexing (leds[i] and points[i] refer to same physical LED)
- Zero-based indexing (required for FastLED and hardware addressing)
- Direct array access for FastLED compatibility
- Point data for geometric calculations and relationships

The key difference is that while the storage remains the same, the access patterns have evolved:
```cpp
// Direct LED access (FastLED compatible)
CRGB& led = model.leds[42];
led.fadeToBlackBy(128);

// Corresponding point access
const Point& point = model.points[42];
float height = point.y();
```

### Access Patterns

The model provides three main patterns for working with LEDs:

1. Direct LED Access (FastLED Compatible):

```cpp
// Get LED reference for direct manipulation
CRGB& led = model.leds[42];
led = CRGB::Blue;
led.fadeToBlackBy(128);

// Or use inline operations
model.leds[42] = CRGB::Red;
model.leds[42].nscale8(192);

// Point access for coordinates
const Point& point = model.points[42];
float x = point.x();
```

2. Collection Operations:

```cpp
// Fill operations
model.leds.fill(CRGB::Blue);

// Iteration with references
for(CRGB& led : model.leds) {
    led.fadeToBlackBy(128);
}

// Point iteration
for(const Point& point : model.points) {
    float height = point.y();
    model.leds[point.id()].setHue(height * 255);
}
```

### Animation Patterns

Different animation styles access LEDs differently:

1. LED-based animations (colors, patterns):

```cpp
// Fill all LEDs blue
model.leds.fill(CRGB::Blue);
```

2. Geometric animations (height, distance):

```cpp
// Work with coordinates
for(size_t i = 0; i < model.leds().size(); i++) {
    float height = model.points[i].y();
    uint8_t brightness = map(height, -1, 1, 0, 255);
    model.leds[i] = CRGB(255-brightness, brightness, brightness/2);
}

// Or using range-based for with points
for(const auto& point : model.points()) {
    // Create a color based on distance from origin
    float distance = sqrt(point.x() * point.x() + point.y() * point.y() + point.z() * point.z());
    model.leds[point.id()] = CRGB::FromHSV(distance * 255, 255, 255);
}
```

3. LedGroup-based animations (user-defined groups):

```cpp
// Fill each ring red
for(auto& ring : model.led_groups("ring0", "ring1", "ring2")) {
    ring.fill(CRGB::Red);
}
// define shapes or symbols, for example counting from 1-3 using led_groups
for (int i=0; i<3; i++) {
    model.led_groups("digit_" + String(i)).fill(CRGB::Red);
}   
```

## Model API

The model interface is designed to allow inspection of the overall shape and its components, as well as manipulation of the LEDs. Many methods are chainable, and return the object they are called on, allowing for a fluid syntax.

### Core Classes

- `Model` - The main model class that implements IModel interface
  - `model_internal.h` - Generated model macros for configuration via generated code
- `Face` - A physical face of the model with its LEDs and layout
  - `FaceType` - Enum defining types of faces (Pentagon, Triangle, etc)
  - `LedGroup` - A collection of LEDs on a face, defined by a list of LED indices
  - `Point` - A 3D point with LED index and face assignment

### Collection Access

Collections are accessed in two ways:

1. Direct element access:
   - model.leds[i] = CRGB::Red
   - model.faces[0].leds[0] = CRGB::Red

2. Collection operations:
   - model.leds().fill(CRGB::Blue)
   - model.faces().size()

3. Led Groups (lists of led indices):
   - model.led_groups("ring0", "ring1", "ring2")
   - model.led_groups("middle")

Collections are returned as `ArrayView` types, and can be iterated over or indexed. By convention, a matching `operator[]` method is offered for indexing each collection type:

- `model.faces()`  // collection of all faces
- `auto& face = model.faces[0]`  // single face

This works in a similar way to the `std::span`, but is specialized for the model classes and provides bounds checking and type safety, as well as cross-platform compatibility with hardware environments that don't support the C++20 standard library (like Teensy, ESP32, etc).

### API Methods

### Model Metadata

- `model.name` - Name of the model
- `model.version` - Version of the model
- `model.description` - Description of the model

### Model Collections

- `model.faces()` - collection of all faces (in order of definition)
- `model.leds()` - collection of all LEDs as CRGB colors
- `model.points()` - collection of all Points

### Faces

given `auto face = model.faces[0];`

- `face.leds()` - collection of all LEDs on the face
- `face.led_offset()` - the global index of the first LED on the face
- `face.points()` - collection of all points on the face
- `face.led_groups()` - collection of all led groups on the face (in order of definition)


### Syntax Examples

We aim to support a clean and easy style of syntax for accessing regionsand manipulating leds on the model.

```cpp
// Direct LED access and index operator
model.leds[2];                          // index getter, Led type
model.leds[42] = CRGB::Blue;            // Set LED color directly
model.leds().size();                    // optional getter syntax, size_t

// Point to LED relationships
float x = model.points[42].x();           // Get x coordinate of LED 42
auto& center = face.led_groups("center");
Point p = model.points[center[0]+face.led_offset()];    // middle Point on the face
for(const auto& id : model.findNearby(p, 0.5f)) {      // find all leds within 0.5 radius
    model.leds[id] = CRGB::Blue;
}

// Iterate faces
for(size_t i = 0; i < model.faces().size(); i++) {
    auto& face = model.faces[i];
    // fill different color for each face
    CHSV hsv = CHSV(i * 255 / model.faces().size(), 255, 255);
    face.leds().fill(hsv);
}
```

All array access and methods should be bounds-checked and not crash on invalid inputs:

- Out of range indices return last valid element
- Invalid geometric queries return zero/empty results
- Color operations on empty spans are safely ignored
- Warnings are logged to the console when invalid operations are attempted

### Setup and Configuration in main.cpp

```cpp
// In main.cpp
#include "FastLED.h"
#include "PixelTheater.h"
#include "models/DodecaRGBv2r0.h"
#include "scenes/test_scene.h"

CRGB leds[Models::DodecaRGBv2r0::NUM_LEDS];   // Define the FastLED-compatible array

void setup() {
    using namespace Models::DodecaRGBv2r0;
    
    FastLED.addLeds<WS2812B, 19, GRB>(leds, 0, NUM_LEDS/2);
    FastLED.addLeds<WS2812B, 18, GRB>(leds + NUM_LEDS/2, NUM_LEDS/2);

    Model model = new Model(ModelDefinition::DodecaRGBv2r0);

    // startup: Set the brightness and fill the LEDs with a color
    stage.brightness(128);
    stage.leds().fill(CRGB::Green);
    stage.update();
    stage.delay(500);

    Stage stage = new Stage(model, leds);
    stage.addScene(new TestScene());
}

void loop() {
    stage.tick();
    stage.update();
}
```

## Model Definition and Generation

### YAML Definition

Models are defined in YAML with explicit IDs and relationships:

```yaml
# Model metadata
name: SampleModel
model_type: Dodecahedron   # this tells the generator what math strategy to use
version: 1.0.0
description: "Sample model for testing"

# Face type definitions
face_types:          # most models will have a single face type, but some may have multiple
  - small_pentagon:
        face_type: Pentagon
        num_leds: 104
        edge_length_mm: 50.0
        led_groups:
            # All indices are local to face
            middle: [0]
            ring0: [1, 2, 3, 4, 5]
            shape: [6, 7, 8, 16, 17, 18]

# Face instances, all conform to face_types. rotation is important for PCB assembly.
faces:
  - id: 0
    type: small_pentagon
    rotation: 0
  - id: 1
    type: small_pentagon
    rotation: 2
  - id: 2
    type: small_pentagon
    rotation: 1
    ## ... etc
    
# LED definitions - all points are generated from the pnp file(s), one for each face type.
led_placement:
  - face_type: small_pentagon
    pnp_file: "pnp-files/dodecav2r1.csv"
```

### Model File Structure

Models are organized in self-contained folders:

```cpp
// Model folder structure:
src/
  models/
    dodecav2r1/             # Each model in its own folder
      README.md             # Model user documentation
      model.cpp             # **Generated data** (using ModelDefinition struct)
      model.yaml            # Model definition
      pcb/                  # PCB files
        pentagon.csv        # Pick and place file
```

The initialization code uses this data to implement the model interface. The generated files are focused purely on the model-specific data, while the API implementation lives in the core library.

### Generation Process

Python generator validates YAML:

- No required values or sections are missing
- Face IDs are unique and sequential (0..N-1)
- LED IDs are unique and sequential (0..N-1)
- LedGroups indices are valid

The generator creates C++ model.h file which is based on the ModelDefinition template. It's a simple struct and some constants that are used to define the data for the model.

### Model Definition Format

The base ModelDefinition provides a template and common constants. NumLeds and NumFaces are required parameters for initialization of FastLED and hardware setup. The rest is generated from the YAML file.

```cpp
template<uint16_t NumLeds, uint8_t NumFaces>
struct ModelDefinition {    
    // Required constants
    static constexpr uint16_t LED_COUNT = NumLeds;
    static constexpr uint8_t FACE_COUNT = NumFaces;

    struct Metadata {
        const char* name;
        const char* model_type;
        const char* version;
        const char* description;
    };

    // Face type constants
    struct FaceTypes {
        static constexpr uint8_t NONE = 0;
        static constexpr uint8_t STRIP = 1;
        static constexpr uint8_t CIRCLE = 2;
        static constexpr uint8_t TRIANGLE = 3;
        static constexpr uint8_t SQUARE = 4;
        static constexpr uint8_t PENTAGON = 5;
        static constexpr uint8_t HEXAGON = 6;
        static constexpr uint8_t HEPTAGON = 7;
        static constexpr uint8_t OCTAGON = 8;
    };

    // Face type properties
    struct FaceTypeData {
        uint8_t id;
        FaceType type;
        uint16_t num_leds;
        float edge_length_mm;
    };

    // Led group data
    struct LedGroupData {
        uint8_t id;
        uint16_t num_leds;
        const char* name[16];
        FaceType type;
        uint16_t led_ids[MAX_LEDS_PER_REGION];
    };


    // Face instance data
    struct FaceData {
        uint8_t id;
        uint8_t type_id;
        uint8_t rotation;
        float x, y, z;  // Position (normal calculated from this)
    };

    // Point geometry
    struct PointData {
        uint16_t id;
        uint8_t face_id;
        float x, y, z;
    };

    // Point neighbor data
    struct NeighborData {
        uint16_t point_id;
        struct Neighbor {
            uint16_t id;
            float distance;
        };
        static constexpr size_t MAX_NEIGHBORS = 7;
        Neighbor neighbors[MAX_NEIGHBORS];
    };

};
```

### Generated Model.h

```cpp
struct ModelDefinition {
    static constexpr uint16_t LED_COUNT = 1248;
    static constexpr uint8_t FACE_COUNT = 12;

    // Model metadata
    static constexpr char NAME[] = "Valid Pentagon Model";
    static constexpr char VERSION[] = "1.0";
    static constexpr char DESCRIPTION[] = "Pre-configured pentagon model with valid regions";

    // Generated data
    static constexpr FaceTypeData face_types[FACE_TYPES_COUNT] = { ... };
    static constexpr FaceData faces[FACE_COUNT] = { ... };
    static constexpr NeighborData neighbors[POINT_COUNT] = { ... };
    static constexpr PointData points[LED_COUNT] = { ... };
};
```

The Model class includes this data at compile-time and provides runtime access through its interface.

### Model Definition Requirements

The model must include metadata, and define at least one face type, and at least one face instance. Coordinates in a model are based on a unit sphere, with the origin at the center. All faces are assumed to be tangent to the origin.

Each face is assumed to have edges of equal length, defined in millimeters. This is helpful for calculating led density and real-world size of the model.

### Index Spaces

The implementation maintains two distinct index spaces:

1. **Global Indices** (0 to LED_COUNT-1)
- Used by Model for LED and Point arrays
- Required for FastLED compatibility
- Used for hardware addressing

2. **Local Indices** (0 to face.num_leds-1)
- Used within Faces and LedGroups
- Makes face-based animations simpler
- Preserves PCB LED sequences

Example:

```cpp
// Global space (model level)
model.leds(42) = CRGB::Blue;

// Local space (face level)
face.leds(3) = CRGB::Red;  // Local index 3

// Converting between spaces
auto i = face.led_groups("shape")[0];
size_t global_idx = face.led_offset() + i;
```
