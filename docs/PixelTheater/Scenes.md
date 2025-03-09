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
├── space.cpp             # Scene implementation 
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
        param("speed", 0.5f, Flags::CLAMP, "Controls animation speed");
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

### Parameter Definition

Parameters are defined in the `setup()` method using the `param()` method:

```cpp
void setup() override {
    // Float parameter with range [0.0, 1.0]
    param("speed", "ratio", 0.5f, "clamp", "Controls animation speed");
    
    // Integer parameter with range [0, 100]
    param("count", "count", 0, 100, 50, "", "Number of particles");
    
    // Boolean parameter
    param("trails", "switch", true, "", "Enable motion trails");
    
    // Float parameter with range [0.0, 2.0]
    param("size", "range", 0.0f, 2.0f, 1.0f, "", "Particle size");
}
```

### Parameter Access

Parameters can be accessed using the settings object:

```cpp
void tick() override {
    Scene::tick();
    
    // Access parameters
    float speed = settings["speed"];
    int count = settings["count"];
    bool trails = settings["trails"];
    
    // Use parameters in animation logic
    if (trails) {
        fadeToBlackBy(leds, 255 * (1.0f - speed));
    } else {
        fill_solid(leds, CRGB::Black);
    }
    
    // Update particles
    for (int i = 0; i < count; i++) {
        updateParticle(i, speed);
    }
}
```

### Parameter Schema

You can generate a schema of all parameters for UI rendering or documentation:

```cpp
// Get parameter schema as JSON
auto schema = scene.parameter_schema().to_json();

// Check if a parameter exists
bool has_speed = scene.has_parameter("speed");

// Get all parameter names
auto names = scene.parameter_names();
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
