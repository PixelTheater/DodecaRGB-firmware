---
category: Development
generated: 2025-02-13 18:48
version: 2.8.3
---

# Scene System

Scenes are the core animation components in PixelTheater. Each scene defines a self-contained animation with its own parameters, state, and rendering logic.

## Scene Organization

A scene consists of:
```
scenes/space/              # Scene root directory
├── space.yaml            # Parameter configuration
├── space.cpp             # Scene implementation 
├── _params.h            # Generated parameters
├── README.md            # Scene documentation
└── props/               # Scene-specific assets
    ├── nebula.bmp       # Bitmap resource
    └── deep_space.pal   # Palette resource
```

## Scene Implementation

### Lifecycle

```cpp
template<typename ModelDef>
class MyScene : public Scene<ModelDef> {
    using Scene = Scene<ModelDef>;
    using Scene::Scene;  // Inherit constructor
    
    void setup() override {
        // Called once when scene becomes active
        // Initialize parameters and state here
        param("speed", "ratio", 0.5f, "clamp");
        _stars.reserve(settings["stars"]);
    }
    
    void tick() override {
        Scene::tick();  // Required: updates frame counter
        
        // Called every frame
        updateState();
        renderFrame();
    }
    
    std::string status() const override {
        return "Frame: " + std::to_string(tick_count());
    }
};
```

### Stage and Model Access

```cpp
void MyScene::tick() {
    Scene::tick();
    
    // Stage access
    auto& leds = this->stage.leds;           // LED array
    auto& model = this->stage.model;         // Model access
    size_t num_leds = model.led_count();     // Total LEDs
    size_t num_faces = model.face_count();   // Total faces
    
    // LED access patterns
    leds[0] = CRGB::Red;                    // Direct indexing
    model.faces[0].leds[0] = CRGB::Blue;    // Through face
    
    // Range-based iteration
    for(auto& led : leds) {
        fadeToBlackBy(led, 128);
    }
    
    // Face-based iteration
    for(auto& face : model.faces) {
        for(auto& led : face.leds) {
            led = CRGB::Green;
        }
    }
    
    // Point-based access
    for(const auto& point : model.points) {
        float height = point.y();
        leds[point.id()] = CHSV(height * 255, 255, 255);
    }
}
```

## Parameter System

### Definition Methods

1. **YAML Definition** (Recommended):
```yaml
# space.yaml
parameters:
  speed:
    type: ratio
    default: 0.5
    description: "Controls animation speed"
    flags: [clamp]
  
  mode:
    type: select
    values: ["orbit", "spiral", "chaos"]
    default: "orbit"
    description: "Particle motion pattern"
```

2. **Code Definition**:
```cpp
void setup() override {
    param("speed", "ratio", 0.5f, "clamp");
    param("mode", "select", "orbit");
}
```

3. **Inheritance**:
```cpp
class DerivedScene : public BaseScene<ModelDef> {
    void setup() override {
        BaseScene::setup();  // Inherit parameters
        param("local_speed", "ratio", 0.5f);
    }
};
```

### Parameter Types and Access

```cpp
void setup() {
    // Type definitions
    param("opacity", "ratio", 0.5f);         // [0.0, 1.0]
    param("balance", "signed_ratio", 0.0f);  // [-1.0, 1.0]
    param("count", "integer", 10);           // Whole number
    param("active", "boolean", true);        // True/false
    param("mode", "select", "orbit");        // Enumerated string
}

void tick() {
    // Type-safe access
    float speed = settings["speed"];     // Returns float
    int count = settings["count"];       // Returns int
    bool active = settings["active"];    // Returns bool
    
    // Safe defaults for invalid access
    float invalid = settings["nonexistent"];  // Returns 0.0f
    int bad_count = settings["bad_count"];    // Returns -1
    bool missing = settings["missing"];       // Returns false
}
```

### Parameter Flags

- `clamp`: Clamps values to valid range
- `wrap`: Wraps values around range boundaries
- `readonly`: Prevents value modification
- `hidden`: Hides from UI but allows programmatic access

Example:
```cpp
settings["speed"] = 1.2f;    // With "clamp": becomes 1.0f
settings["angle"] = 1.5f;    // With "wrap": wraps to -0.5f
```

## Best Practices

1. **Scene Design**:
   - One scene per file
   - Clear parameter documentation
   - Meaningful scene names
   - Call base class `tick()` for frame counting

2. **Performance**:
   - Minimize allocations in tick()
   - Cache frequently used values
   - Use range-based for loops
   - Pre-allocate vectors in setup()

3. **Error Handling**:
   - Check parameter existence
   - Use safe defaults
   - Validate ranges

See [Stage Documentation](Stage.md) for scene registration and lifecycle management.
