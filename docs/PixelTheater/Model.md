# Model Initialization and Use

A geometric model with LED mapping. Models can be generated from hardware specifications (YAML + pick-and-place files) using the utility scripts.

## Core Relationships

The `Model` contains a collection of `Face` objects and `Led` objects.

Each `Face` represents a physical surface and contains:

- Multiple `Region` objects, grouping LEDs into logical areas (center, rings, edges).
- A `center` region, which is a single `Region` representing the central LEDs.
- `rings`: a collection of `Region` objects representing concentric rings of LEDs.
- `edges`: a collection of `Region` objects representing the edges of the face.
Each `Region` contains:
- An `LedSpan` representing the LEDs within that region.
- Geometric information like a center `Point`.
  
## Model Classes

### Core Classes

- `Model` - The main model class that implements IModel interface
- `model_internal.h` - Generated model macros for configuration via generated code

### Geometry Classes

- `Face` - A physical face of the model with its LEDs and layout
- `FaceType` - Enum defining types of faces (Pentagon, Triangle, etc)
- `Point` - A 3D point with LED index and face assignment

### Region Classes

- `Region` - Groups of LEDs on a face (center, rings, edges)
- `RegionType` - Enum defining types of LED groups (Center, Ring, Edge)
- `Led` - A aggregate LED object that provides access to a CRGB color and Point object
- `LedSpan` - An array of non-contiguous LEDs that form a region


## Model API: A brief list of geometry access methods

Note that collections are returned as `std:array` types, and can be iterated over or indexed. By convention, a matching `operator[]` method is offered for indexing each collection type:

- `model.faces()  // collection of all faces`
- `model.faces[0] // single face`

### Model

Metadata:

- `model.name` - Name of the model
- `model.version` - Version of the model
- `model.description` - Description of the model

### Collections:

- `model.faces()` - collection of all faces (in order of definition)
- `model.regions()` - collection of all regions
- `model.edges()` - collection of all edges
- `model.rings()` - collection of all rings
- `model.centers()` - collection of all center regions
- `model.leds()` - collection of all LEDs
- `model.points()` - collection of all points

### Faces

given `auto face = model.faces[0];`

- `face.edges()` - collection of all edges on the face
- `face.rings()` - collection of all rings on the face
- `face.center()` - Central region of the face
- `face.midpoint()` - Geometric center point of the face
- `face.leds()` - collection of all LEDs on the face
- `face.regions()` - collection of all regions on the face

### Region Components

given `auto edge = face.edges[0];`

- `edge.leds` - collection of all LEDs on the edge
- `edge.midpoint()` - Geometric center point of the edge
- `edge.normal` - Normal vector to the edge plane
- `edge.face` - Face that the edge belongs to
- `edge.type` - The region type (e.g. Edge)

The `leds` property is an LedSpan instance. It can be accessed as an array or iterated over, or inspected to find the properties of the region. As it contains CRGB objects, and can be manipulated like a FastLED array.

### Syntax Examples

We aim to support a clean and easy style of syntax for accessing regionsand manipulating leds on the model.

```cpp
// Direct LED access and index operator, without needing to type model.leds() each time
auto model = stage.model;
model.leds[2];                          // index getter, Led type
model.leds[42] = CRGB::Blue;            // index setter, void
model.leds[42].color = CRGB::Blue;      // set color, CRGB type
model.leds().size();                    // optional getter syntax, size_t

// Face and region access
auto& face = model.faces[3];            // get face by index (Face instance)
auto& ring = face.rings[2];             // get ring by index (Region instance, type FaceType::RING)

// LED batch manipulation
face.rings[2].leds.fill(CRGB::Red);       // Fill third ring
Face f = face.center();                   // get center region (Region of type FaceType::CENTER)
f.center().leds.fill(CRGB::White);        // Fill center LED

// Point to LED relationships
float x = model.leds[42].point().x();     // get x coordinate of LED 42
Point p = face.center().point();          // Get face center point (Point instance)
auto nearby = model.findNearby(p, 0.5f);  // find all leds within 0.5 radius
nearby.fill(CRGB::Blue);

// Iteration with geometry
for(auto& face : model.faces) {
    // Color faces based on height
    float y = face.center().y();
    uint8_t hue = map(y, -50, 50, 0, 255);
    face.leds().fill(CHSV(hue, 255, 255));
    
    // Fade rings by index
    for(int r = 0; r < face.rings().size(); r++) {
        face.rings[r].leds.
            .fill(CHSV(hue, 255, 255))
            .fadeToBlackBy(r * 32);  // Outer rings darker
    }
}
```

All collection types (faces, regions, LEDs, etc.) provide both a faces() getter method as well as an overload of operator[] for convenience. Note that if an out-of-range index is provided, the last valid element is returned.

The Led class provides a very flexible interface, with FastLED style assignment and chaining, which still allows inspection of properties like the led index and point position:

```cpp
auto& led = face.edges[0].leds[0];
led = CRGB::Red;
led.color = CRGB::Red;  // still works!
led.index; // 0
led.point(); // Point
```

All array access and methods should be bounds-checked and not crash on invalid inputs:

- Out of range indices return last valid element
- Invalid geometric queries return zero/empty results
- Color operations on empty spans are safely ignored
- Warnings are logged to the console when invalid operations are attempted

### Generated Model Files

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
    stage.leds.fill(CRGB::Green);
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
      regions:
        centers: 1
        rings: 3
        edges: 5

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
    
# LED definitions - all points are generated from the pnp file(s), one for each face type.
led_placement:
  - face_type: small_pentagon
    pnp_file: "pnp-files/dodecav2r1.csv"
```

### Generation Process

Python generator validates YAML:

- No required values or sections are missing
- Face IDs are unique and sequential (0..N-1)
- LED IDs are unique and sequential (0..N-1)
- Region LED lists reference valid LED IDs
- Face type and region counts match declarations

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
        uint8_t num_centers;
        uint8_t num_rings;
        uint8_t num_edges;
    };

    // Face instance data
    struct FaceData {
        uint8_t id;
        uint8_t type_id;
        uint8_t rotation;
        float x, y, z;  // Position (normal calculated from this)
    };

    // Region type constants  
    struct RegionTypes {
        static constexpr uint8_t CENTER = 0;
        static constexpr uint8_t RING = 1;
        static constexpr uint8_t EDGE = 2;
        static constexpr uint8_t WEDGE = 3;
        static constexpr uint8_t PATCH = 4;
    };

    // Region definition
    struct RegionData {
        uint8_t id;
        uint8_t face_id;
        RegionType type;
        uint16_t led_ids[MAX_LEDS_PER_REGION];
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

Then models can use these constants by including the generated model.h file.

- All generated data for points, regions, and faces is stored in the ModelDefinition struct.
- Regions belong to a Face, and regions in the ModelDefinition have a face_id.
- Every Point belongs to a face, as points have a defined face_id.

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
    static constexpr RegionData regions[REGION_COUNT] = { ... };
    static constexpr NeighborData neighbors[POINT_COUNT] = { ... };
    static constexpr PointData points[LED_COUNT] = { ... };
};
```

The Model class includes this data at compile-time and provides runtime access through its interface.

### Model Definition Requirements

The model must include metadata, and define at least one face type, and at least one face instance. Coordinates in a model are based on a unit sphere, with the origin at the center. All faces are assumed to be tangent to the origin. 

Each face must have a minimum region structure defined for proper model operation:

1. Center Region (Required)
   - Single center point for geometric calculations
   - Reference point for face orientation, ised to establish face normal vectors

2. Ring Regions (Required)
   - Concentric rings around center allow orientation-independent animations
   - Helps to understand the overall resolution of the model (more rings = higher resolution)
   - Enable systematic LED testing patterns, used for hardware testing and validation

3. Edge Regions (Required)
   - Define face boundaries
   - Used for edge-based animations
   - Essential for verifying PCB rotation
   - Valuable feedback during assembly and testing
   - Number of edges is determined by the face type, and is defined in the FaceTypeData struct.

Every LED on a face must be a member of at least one region and define a point. Neighbors are optional, but recommended for more complex animations. If they are not present, some animations may not work properly.

