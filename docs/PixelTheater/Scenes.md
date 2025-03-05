---
category: Development
generated: 2025-02-13 18:48
version: 2.8.3
---

# Scene System

Scenes are the core animation components in PixelTheater. Each scene defines a self-contained animation with its own parameters, state, and rendering logic.

## Scene Structure

A scene consists of:
1. Scene class implementation
2. Parameter definitions (YAML or code)
3. Optional resources (palettes, patterns)
4. Documentation

Example directory structure:
```bash
scenes/space/              # Scene root directory
├── space.yaml            # Parameter configuration
├── space.cpp             # Scene implementation 
├── _params.h            # Generated parameters
├── README.md            # Scene documentation
└── props/               # Scene-specific assets
    ├── nebula.bmp       # Bitmap resource
    └── deep_space.pal   # Palette resource
```

## Basic Scene Implementation

```cpp
namespace PixelTheater {
namespace Scenes {

template<typename ModelDef>
class SpaceScene : public Scene<ModelDef> {
    using Scene = Scene<ModelDef>;
    using Scene::Scene;  // Inherit constructor
    
    void setup() override {
        // Define parameters
        param("speed", "ratio", 0.5f, "clamp");
        param("fade", "integer", 2);
        param("stars", "integer", 100);
        
        // Initialize scene state
        _stars.reserve(settings["stars"]);
        for (int i = 0; i < settings["stars"]; i++) {
            _stars.emplace_back(createStar());
        }
    }
    
    void tick() override {
        Scene::tick();  // Call base to increment counter
        
        // Get current parameter values
        float speed = settings["speed"];
        int fade = settings["fade"];
        
        // Update scene state
        updateStars(speed);
        
        // Apply global effects
        fadeFrame(fade);
        
        // Render the frame
        renderStars();
    }
    
    std::string status() const override {
        return "Stars: " + std::to_string(_stars.size()) + 
               ", Speed: " + std::to_string(settings["speed"]);
    }

private:
    std::vector<Star> _stars;
    
    void updateStars(float speed) {
        for (auto& star : _stars) {
            star.update(speed);
        }
    }
    
    void fadeFrame(int amount) {
        for (auto& led : this->stage->model->leds) {
            fadeToBlackBy(led, amount);
        }
    }
    
    void renderStars() {
        for (const auto& star : _stars) {
            auto& led = this->stage->model->leds[star.led_index];
            led = star.color;
        }
    }
};

}} // namespace PixelTheater::Scenes
```

## Parameter System

Parameters in PixelTheater use a four-layer architecture for type safety and validation:

1. **Scene Layer** (User Interface)
   - Type-safe parameter access via `settings[]`
   - Parameter definition in `setup()`
   - Parameter lifecycle and inheritance

2. **Settings Layer** (Parameter Management)
   - Parameter definitions and values
   - Validation chains
   - Parameter inheritance
   - Bounds checking

3. **Parameter Layer** (Type System)
   - Parameter definitions and metadata
   - Type-safe value container
   - Validation and conversion rules

4. **Handler Layer** (Core Logic)
   - Type validation and conversion
   - Range validation
   - Flag handling
   - Error handling

### Parameter Definition

Parameters can be defined in three ways:

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

2. **Manual Definition**:
```cpp
void setup() override {
    param("speed", "ratio", 0.5f, "clamp");
    param("mode", "select", "orbit");
}
```

3. **Inheritance**:
```cpp
class BaseScene : public Scene<ModelDef> {
    void setup() override {
        param("global_speed", "ratio", 1.0f);
    }
};

class DerivedScene : public BaseScene {
    void setup() override {
        BaseScene::setup();  // Inherit parameters
        param("local_speed", "ratio", 0.5f);
    }
};
```

### Parameter Types

The parameter system supports these types:

1. **ratio**: Float value [0.0, 1.0]
   ```cpp
   param("opacity", "ratio", 0.5f, "clamp");
   float opacity = settings["opacity"];  // Range [0.0, 1.0]
   ```

2. **signed_ratio**: Float value [-1.0, 1.0]
   ```cpp
   param("balance", "signed_ratio", 0.0f);
   float balance = settings["balance"];  // Range [-1.0, 1.0]
   ```

3. **integer**: Whole number value
   ```cpp
   param("count", "integer", 10);
   int count = settings["count"];
   ```

4. **boolean**: True/false value
   ```cpp
   param("active", "boolean", true);
   bool active = settings["active"];
   ```

5. **select**: Enumerated string value
   ```cpp
   param("mode", "select", "orbit");
   std::string mode = settings["mode"];
   ```

### Parameter Access

Parameters are accessed through the type-safe `settings[]` interface:

```cpp
void SpaceScene::tick() {
    // Type-safe access with validation
    float speed = settings["speed"];     // Returns float
    int count = settings["count"];       // Returns int
    bool active = settings["active"];    // Returns bool
    
    // Invalid access returns safe sentinel values
    float invalid = settings["nonexistent"];  // Returns 0.0f
    int bad_count = settings["bad_count"];    // Returns -1
    bool missing = settings["missing"];       // Returns false
    
    // Values are automatically clamped/wrapped based on flags
    settings["speed"] = 1.2f;    // With "clamp": becomes 1.0f
    settings["angle"] = 1.5f;    // With "wrap": wraps to -0.5f
}
```

### Parameter Flags

Parameters can have flags that modify their behavior:

- `clamp`: Clamps values to valid range
- `wrap`: Wraps values around range boundaries
- `readonly`: Prevents value modification
- `hidden`: Hides from UI but allows programmatic access

## LED Access Patterns

Scenes can access LEDs through the model in several ways:

1. **Direct LED Access**:
```cpp
// Single LED access
auto& led = this->stage->model->leds[0];
led = CRGB::Red;

// Range-based iteration
for (auto& led : this->stage->model->leds) {
    fadeToBlackBy(led, 128);
}
```

2. **Face-based Access**:
```cpp
// Single face
auto& face = this->stage->model->faces[0];
for (auto& led : face.leds) {
    led = CRGB::Blue;
}

// All faces
for (auto& face : this->stage->model->faces) {
    for (auto& led : face.leds) {
        led = CRGB::Green;
    }
}
```

3. **Point-based Access**:
```cpp
// Access by point
for (const auto& point : this->stage->model->points) {
    float height = point.y();
    this->stage->model->leds[point.id()] = CHSV(height * 255, 255, 255);
}
```

## Best Practices

1. **Scene Organization**:
   - One scene per file
   - Clear parameter documentation
   - Meaningful scene names

2. **Parameter Management**:
   - Use YAML for parameter definition
   - Document parameter ranges
   - Use appropriate parameter types

3. **Performance**:
   - Minimize allocations in tick()
   - Cache frequently used values
   - Use range-based for loops

4. **Error Handling**:
   - Check parameter existence
   - Validate parameter ranges
   - Use safe defaults

See [Stage Documentation](Stage.md) for details on scene registration and lifecycle management.
