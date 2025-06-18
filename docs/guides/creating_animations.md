---
category: Guide, Development
version: 2.8.2
---

# Creating Animations Guide

This comprehensive guide covers creating animation scenes for the PixelTheater library, from basic concepts to advanced techniques.

## Prerequisites

- Familiarity with C++ and the development environment
- Understanding of the [Scene API Reference](../PixelTheater/Scenes.md)
- Development environment set up per [Development Guide](development.md)

## What is a Scene?

A Scene represents a single, self-contained animation or visual effect. It manages its own state, parameters, and logic for calculating LED colors based on the model geometry and time. Scenes are portable across different PixelTheater hardware models and the [web simulator](web-simulator.md).

Scenes are managed and run by the main `Theater` class, which handles switching between scenes, providing access to the LED array and 3D model data, and managing the animation loop.

## Part 1: Getting Started

### Project Structure

Create your scene files in an organized structure:

```
src/scenes/
├── my_scene/
│   ├── my_scene.h          # Scene class declaration
│   ├── my_scene.cpp        # Scene implementation (optional)
│   ├── poster.png          # Visual preview (optional)
│   └── README.md           # Description (optional)
└── other_scenes/
```

### Creating Your First Scene

Your scene class must inherit from `PixelTheater::Scene` and implement required methods:

```cpp
#pragma once
#include "PixelTheater.h"

// Bring PixelTheater namespaces into scope
using namespace PixelTheater;
using namespace PixelTheater::Constants; // For PT_PI, PT_TWO_PI etc.

namespace Scenes {

class MyScene : public Scene {
public:
    MyScene() = default;
    ~MyScene() override = default;

    // Called once before the scene runs for the first time
    void setup() override;

    // Called repeatedly for each animation frame
    void tick() override;

    // Optional: Called when switching back to this scene
    // void reset() override { Scene::reset(); /* + custom reset */ }

private:
    float _phase = 0.0f;
    // ... other state variables ...
};

} // namespace Scenes
```

**Key Points:**
- Include `PixelTheater.h`
- Use the `Scenes` namespace (recommended)
- Inherit publicly from `PixelTheater::Scene`
- Override the required `setup()` and `tick()` methods

### Scene Setup

Use `setup()` for initialization, metadata, and parameter definitions:

```cpp
void MyScene::setup() {
    // 1. Set Metadata (important for UI/logging)
    set_name("My First Scene");
    set_description("A simple animation example");
    set_author("Your Name");
    set_version("1.0");
    
    // Alternative metadata approach
    // meta("source", "https://github.com/...");
    
    // 2. Define Parameters for user control
    param("speed", "ratio", 0.5f, "clamp", "Animation speed");
    param("brightness", "range", 0.1f, 1.0f, 0.8f, "", "LED brightness");
    param("color_hue", "count", 0, 255, 128, "wrap", "Base color hue");
    param("enabled", "switch", true, "", "Enable/disable effect");
    
    // 3. Initialize scene state
    _phase = randomFloat(0.0f, PT_TWO_PI);
}
```

**Parameter Types:**
- `ratio`: 0.0-1.0 range with percentage display
- `range`: Custom min/max range
- `count`: Integer range (good for hue values 0-255)
- `switch`: Boolean on/off

### Basic Animation Loop

Implement your animation logic in `tick()`:

```cpp
void MyScene::tick() {
    Scene::tick(); // Call base class first
    
    // Get user-configured parameters
    float speed = settings["speed"];
    float brightness = settings["brightness"];
    int hue = settings["color_hue"];
    
    if (!settings["enabled"]) {
        // Fade out when disabled
        for (size_t i = 0; i < ledCount(); ++i) {
            leds[i].fadeToBlackBy(32);
        }
        return;
    }
    
    // Update animation state (frame-rate independent)
    _phase += speed * deltaTime() * PT_TWO_PI;
    if (_phase > PT_TWO_PI) _phase -= PT_TWO_PI;
    
    // Set LED colors
    for (size_t i = 0; i < ledCount(); ++i) {
        uint8_t wave = sin8(_phase * 255 / PT_TWO_PI + i * 10);
        uint8_t scaledBrightness = brightness * wave;
        leds[i] = CHSV(hue, 255, scaledBrightness);
    }
}
```

### Adding Your Scene to the Theater

Include your scene in `src/main.cpp`:

```cpp
#include "PixelTheater.h"
#include "models/DodecaRGBv2_1/model.h"
#include "scenes/my_scene/my_scene.h"

PixelTheater::Theater theater;

void setup() {
    // Initialize platform and model
    theater.useFastLEDPlatform<DodecaRGBv2_1Model>(leds, NUM_LEDS);
    
    // Add your scene
    theater.addScene<Scenes::MyScene>();
    
    // Add other scenes...
    theater.addScene<Scenes::SparklesScene>();
    
    theater.start();
}

void loop() {
    theater.update(); // Update current scene and display
}
```

## Part 2: LED Management

### LED Access Patterns

**Direct Index Access:**
```cpp
leds[0] = CRGB::Red;               // Set specific LED
leds[i].fadeToBlackBy(32);         // Fade LED towards black
leds[i].nscale8(128);              // Scale to 50% brightness
```

**Range Operations:**
```cpp
// Fill range with color
CRGB fillColor = CRGB::Blue;
for (size_t i = 0; i < 10 && i < ledCount(); ++i) {
    leds[i] = fillColor;
}

// Fade all LEDs
for (size_t i = 0; i < ledCount(); ++i) {
    leds[i].fadeToBlackBy(15);
}
```

**Bounds Safety:**
```cpp
// LEDs are automatically bounds-checked
size_t index = 9999; // Won't crash even if > ledCount()
leds[index] = CRGB::Green; // Safe access (clamped to valid range)
```

### Color Systems

**RGB vs HSV:**
```cpp
// RGB - Direct color values
leds[i] = CRGB(255, 128, 0);      // Orange
leds[i] = CRGB::Purple;           // Named color

// HSV - Hue, Saturation, Value (easier for animations)
leds[i] = CHSV(160, 255, 200);    // Hue=160°, full sat, 78% brightness
leds[i].setHue(hue);              // Change just hue
leds[i].setValue(brightness);      // Change just brightness
```

**Color Blending:**
```cpp
CRGB targetColor = CRGB::Yellow;
leds[i] = blend(leds[i], targetColor, 64); // 25% blend towards yellow
nblend(leds[i], targetColor, 128);         // 50% blend (modifies in-place)
```

## Part 3: Using Palettes

Palettes provide sophisticated color schemes:

```cpp
void MyScene::tick() {
    Scene::tick();
    
    // Get palette (use PixelTheater::Palettes namespace)
    const CRGBPalette16& palette = Palettes::RainbowColors;
    
    // Animate index over time
    uint8_t paletteIndex = millis() / 20; // Slowly cycle through palette
    
    for (size_t i = 0; i < ledCount(); ++i) {
        // Offset index per LED for spatial variation
        uint8_t ledIndex = paletteIndex + (i * 8);
        
        // Sample palette with brightness and blending
        leds[i] = colorFromPalette(palette, ledIndex, 200, LINEARBLEND);
    }
}
```

**Available Palettes:**
- `Palettes::RainbowColors` - Full spectrum
- `Palettes::LavaColors` - Red/orange/yellow
- `Palettes::OceanColors` - Blue/green/white  
- `Palettes::PartyColors` - Vibrant party colors

## Part 4: Coordinate Systems

### Face-Based Rendering

Access LEDs grouped by physical faces:

```cpp
void MyScene::tick() {
    Scene::tick();
    const IModel& model_ref = model();
    
    for (size_t faceIdx = 0; faceIdx < model_ref.faceCount(); ++faceIdx) {
        // Get face proxy for clean access
        auto face_proxy = model_ref.face(faceIdx);
        
        // Color based on face index
        CRGB faceColor = CHSV(faceIdx * 30, 255, 200);
        
        // Access LEDs in this face
        auto faceLeds = face_proxy.leds();
        for (auto& led : faceLeds) {
            led = faceColor;
        }
        
        // Or access specific LED groups
        auto centerLeds = face_proxy.group("center");
        for (auto& led : centerLeds) {
            led = CRGB::White; // Highlight center
        }
    }
}
```

### 3D Coordinate Access

Use 3D positions for spatial effects:

```cpp
void MyScene::tick() {
    Scene::tick();
    const IModel& model_ref = model();
    float time = millis() / 1000.0f;
    
    for (size_t i = 0; i < ledCount(); ++i) {
        const Point& p = model_ref.point(i);
        
        // Map Z height to brightness
        float sphereRadius = model_ref.getSphereRadius();
        float normalizedZ = (p.z() / sphereRadius + 1.0f) / 2.0f; // -1..1 → 0..1
        
        // Create moving wave
        float wave = sin(normalizedZ * PT_TWO_PI * 3.0f + time * 2.0f);
        uint8_t brightness = (wave + 1.0f) * 127.5f; // -1..1 → 0..255
        
        leds[i] = CHSV(120, 255, brightness); // Green wave
    }
}
```

### Neighbor-Based Effects

Use connectivity for spreading effects:

```cpp
void MyScene::tick() {
    Scene::tick();
    
    // Spark effect that spreads to neighbors
    if (random8() < 10) { // 10/255 chance per frame
        size_t sparkLed = random16(ledCount());
        leds[sparkLed] = CRGB::White;
        
        // Spread to neighbors
        const Point& p = model().point(sparkLed);
        const auto& neighbors = p.getNeighbors();
        for (const auto& neighbor : neighbors) {
            if (neighbor.id < ledCount() && neighbor.distance > 0.0f) {
                leds[neighbor.id] = CHSV(60, 255, 128); // Dimmer yellow
            }
        }
    }
    
    // Fade all LEDs
    for (size_t i = 0; i < ledCount(); ++i) {
        leds[i].fadeToBlackBy(8);
    }
}
```

## Part 5: Time and Animation

### Frame-Rate Independent Animation

Always use `deltaTime()` for smooth animation:

```cpp
void MyScene::tick() {
    Scene::tick();
    float speed = settings["speed"];
    
    // Wrong - framerate dependent
    _position += velocity; 
    
    // Correct - framerate independent  
    _position += velocity * speed * deltaTime();
    
    // Wrap position
    if (_position > 1.0f) _position -= 1.0f;
}
```

### Timing Utilities

**Absolute Time:**
```cpp
uint32_t time_ms = millis();
float time_seconds = time_ms / 1000.0f;

// Periodic effects
float cycle = fmod(time_seconds / 3.0f, 1.0f); // 0-1 over 3 seconds
uint8_t brightness = sin8(cycle * 255); // Sine wave brightness
```

**Relative Time:**
```cpp
size_t frame = tickCount(); // Frames since scene started
float dt = deltaTime();     // Seconds since last frame

// Accumulate time
static float accumulated_time = 0.0f;
accumulated_time += dt;
```

### Smooth Transitions

**Easing Functions:**
```cpp
// Linear interpolation
float progress = fmod(millis() / 2000.0f, 1.0f); // 0-1 over 2 seconds
uint8_t brightness = lerp8by8(50, 255, progress * 255); // 50→255 over time

// Sine easing for smooth motion
float eased = (sin(progress * PT_PI - PT_PI/2) + 1.0f) / 2.0f; // 0-1 smooth
```

**Color Transitions:**
```cpp
CRGB startColor = CRGB::Red;
CRGB endColor = CRGB::Blue; 
float blend_amount = sin8(millis() / 10) / 255.0f; // 0-1 oscillation
CRGB currentColor = blend(startColor, endColor, blend_amount * 255);
```

## Part 6: Advanced Techniques

### Multi-Layer Effects

Combine multiple effects for rich visuals:

```cpp
void MyScene::tick() {
    Scene::tick();
    
    // Layer 1: Background gradient
    for (size_t i = 0; i < ledCount(); ++i) {
        const Point& p = model().point(i);
        float height = (p.z() / model().getSphereRadius() + 1.0f) / 2.0f;
        leds[i] = CHSV(200, 255, height * 100); // Blue gradient
    }
    
    // Layer 2: Moving sparkles
    uint8_t sparkleIndex = millis() / 50;
    for (size_t i = 0; i < ledCount(); ++i) {
        uint8_t noise = inoise8(i * 30, sparkleIndex);
        if (noise > 240) { // Rare sparkles
            leds[i] += CRGB(80, 80, 80); // Add white
        }
    }
    
    // Layer 3: Breathing effect
    uint8_t breath = sin8(millis() / 30);
    for (size_t i = 0; i < ledCount(); ++i) {
        leds[i].nscale8(breath);
    }
}
```

### Physics Simulation

Simple particle system:

```cpp
struct Particle {
    Vector3f position;
    Vector3f velocity;
    CRGB color;
    float life = 1.0f;
};

class PhysicsScene : public Scene {
private:
    std::vector<Particle> particles;
    
public:
    void tick() override {
        Scene::tick();
        float dt = deltaTime();
        
        // Update particles
        for (auto& p : particles) {
            p.position += p.velocity * dt;
            p.life -= dt * 0.5f; // Fade over 2 seconds
            
            // Find closest LED
            size_t closestLed = 0;
            float minDist = 1e6f;
            for (size_t i = 0; i < ledCount(); ++i) {
                const Point& point = model().point(i);
                Vector3f ledPos(point.x(), point.y(), point.z());
                float dist = (ledPos - p.position).norm();
                if (dist < minDist) {
                    minDist = dist;
                    closestLed = i;
                }
            }
            
            // Light up closest LED
            CRGB scaledColor = p.color;
            scaledColor.nscale8(p.life * 255);
            leds[closestLed] += scaledColor;
        }
        
        // Remove dead particles
        particles.erase(
            std::remove_if(particles.begin(), particles.end(),
                [](const Particle& p) { return p.life <= 0.0f; }),
            particles.end()
        );
        
        // Add new particles occasionally
        if (random8() < 30) {
            Particle newParticle;
            newParticle.position = Vector3f(
                randomFloat(-100, 100),
                randomFloat(-100, 100), 
                randomFloat(-100, 100)
            );
            newParticle.velocity = Vector3f(
                randomFloat(-50, 50),
                randomFloat(-50, 50),
                randomFloat(-50, 50)
            );
            newParticle.color = CHSV(random8(), 255, 255);
            particles.push_back(newParticle);
        }
    }
};
```

### Performance Optimization

**Pre-calculate when possible:**
```cpp
class OptimizedScene : public Scene {
private:
    std::vector<float> heightFactors; // Pre-calculated
    
public:
    void setup() override {
        Scene::setup();
        
        // Pre-calculate height factors
        heightFactors.resize(ledCount());
        float radius = model().getSphereRadius();
        for (size_t i = 0; i < ledCount(); ++i) {
            const Point& p = model().point(i);
            heightFactors[i] = (p.z() / radius + 1.0f) / 2.0f;
        }
    }
    
    void tick() override {
        Scene::tick();
        
        // Use pre-calculated values
        for (size_t i = 0; i < ledCount(); ++i) {
            uint8_t brightness = heightFactors[i] * 255;
            leds[i] = CHSV(120, 255, brightness);
        }
    }
};
```

**Limit expensive operations:**
```cpp
void MyScene::tick() {
    Scene::tick();
    
    // Only update every 5th frame for expensive calculations
    if (tickCount() % 5 == 0) {
        // Expensive noise calculation
        for (size_t i = 0; i < ledCount(); i += 5) { // Sample subset
            noiseValues[i] = inoise8(i * 20, millis() / 50);
        }
    }
    
    // Use cached values for lighting
    for (size_t i = 0; i < ledCount(); ++i) {
        uint8_t noise = noiseValues[i / 5 * 5]; // Use nearest sample
        leds[i] = CHSV(noise, 255, 200);
    }
}
```

## Part 7: Available API Reference

These methods are available within your Scene class:

### LEDs
- `leds[index]` - Access LED with automatic bounds checking
- `led(index)` - Alternative LED access method
- `ledCount()` - Total number of LEDs

### Model/Geometry
- `model()` - Get reference to model interface
- `model().point(index)` - Get 3D point data for LED
- `model().face(index)` - Get face data
- `model().pointCount()` - Number of points (equals ledCount)
- `model().faceCount()` - Number of faces

### Parameters/Settings
- `param(...)` - Define parameters in `setup()`
- `meta(key, value)` - Define metadata in `setup()`
- `settings["name"]` - Access/modify parameter values
- `name()`, `description()`, `version()`, `author()` - Get scene metadata

### Utilities
- `millis()`, `deltaTime()`, `tickCount()` - Timing
- `random8()`, `random16()`, `randomFloat()` - Random numbers
- `logInfo()`, `logWarning()`, `logError()` - Logging

## Best Practices Summary

1. **Use `deltaTime()` for frame-rate independence**
2. **Pre-calculate expensive operations in `setup()`**
3. **Use palettes for sophisticated color schemes**
4. **Leverage model geometry for spatial effects**
5. **Test in web simulator before hardware deployment**
6. **Use parameters for user control**
7. **Follow the layer pattern for complex effects**
8. **Profile performance for smooth animation**

## See Also

- [Scene API Reference](../PixelTheater/Scenes.md) - Complete API documentation
- [Parameters Guide](../PixelTheater/Parameters.md) - Parameter system details
- [Model System](../PixelTheater/Model.md) - Geometry and face access
- [Development Guide](development.md) - Environment setup and tools 