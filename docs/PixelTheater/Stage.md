# Stage System

The Stage is the central orchestrator in PixelTheater, managing the hardware platform, model state, and scene execution. It provides a unified interface for coordinating all components of an LED animation system.

## Core Responsibilities

1. **Hardware Integration**
   - Platform abstraction (FastLED, custom drivers)
   - LED buffer management
   - Frame rate control
   - Hardware synchronization

2. **Scene Management**
   - Scene registration and lifecycle
   - Scene transitions
   - Global state coordination
   - Resource allocation

3. **Update Loop**
   - Frame timing
   - Scene execution
   - LED buffer updates
   - Platform synchronization

## Basic Setup

```cpp
#include "FastLED.h"
#include "PixelTheater.h"
#include "models/DodecaRGBv2/model.h"  // Include generated model header

// Define model type alias - all models live in PixelTheater::Models namespace
using DodecaModel = PixelTheater::Models::DodecaRGBv2;

class Application {
private:
    // LED buffer for FastLED
    CRGB leds[DodecaModel::LED_COUNT];
    
    // Core components
    std::unique_ptr<PixelTheater::FastLEDPlatform> platform;
    std::unique_ptr<PixelTheater::Model<DodecaModel>> model;
    std::unique_ptr<PixelTheater::Stage<DodecaModel>> stage;

public:
    void setup() {
        // Initialize FastLED
        FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, DodecaModel::LED_COUNT);
        FastLED.setBrightness(128);
        FastLED.setMaxRefreshRate(60);
        
        // Create platform
        platform = std::make_unique<PixelTheater::FastLEDPlatform>(
            reinterpret_cast<PixelTheater::CRGB*>(leds),
            DodecaModel::LED_COUNT
        );
        
        // Create model
        model = std::make_unique<PixelTheater::Model<DodecaModel>>(
            DodecaModel{},
            platform->getLEDs()
        );
        
        // Create stage
        stage = std::make_unique<PixelTheater::Stage<DodecaModel>>(
            std::move(platform),
            std::move(model)
        );
        
        // Register scenes
        registerScenes();
    }
    
    void loop() {
        stage->update();  // Updates current scene and syncs hardware
    }

private:
    void registerScenes() {
        // Register scenes with metadata
        stage->registerScene<SpaceScene<DodecaModel>>(
            "Space",
            "Deep space visualization"
        );
        
        stage->registerScene<FireworksScene<DodecaModel>>(
            "Fireworks",
            "Colorful firework display"
        );
        
        // Set initial scene
        if (auto* scene = stage->getScene("Space")) {
            stage->setScene(scene);
        }
    }
};
```

## Platform Integration

The Stage manages hardware through platform abstractions:

```cpp
// Platform interface (simplified)
class Platform {
public:
    virtual void update() = 0;              // Update hardware
    virtual void setBrightness(uint8_t) = 0;// Set global brightness
    virtual CRGB* getLEDs() = 0;            // Get LED buffer
    virtual size_t getLEDCount() = 0;       // Get LED count
};

// FastLED implementation
class FastLEDPlatform : public Platform {
public:
    void update() override {
        FastLED.show();  // Update physical LEDs
    }
    
    void setBrightness(uint8_t value) override {
        FastLED.setBrightness(value);
    }
    // ... other methods ...
};
```

## Scene Management

The Stage provides a type-safe API for scene management:

```cpp
// Scene registration
auto* scene = stage->registerScene<MyScene<ModelDef>>(
    "My Scene",
    "Scene description"
);

// Scene transitions
stage->setScene(scene);           // Switch to scene
stage->setScene("My Scene");      // Switch by name
stage->nextScene();               // Advance to next scene
stage->previousScene();           // Return to previous scene

// Scene queries
if (auto* scene = stage->getScene("My Scene")) {
    // Scene exists
}

// Scene enumeration
for (const auto& info : stage->getSceneInfo()) {
    std::cout << info.name << ": " << info.description << "\n";
}
```

## Update Loop

The Stage's update loop coordinates all components:

```cpp
void Stage::update() {
    // Update timing
    auto now = std::chrono::steady_clock::now();
    float delta = std::chrono::duration<float>(now - _last_update).count();
    _last_update = now;
    
    // Update current scene
    if (_current_scene) {
        _current_scene->tick();
    }
    
    // Update platform
    _platform->update();
}
```

## Global State

The Stage maintains global state accessible to all scenes:

```cpp
// Global brightness
stage->brightness(128);           // Set brightness
uint8_t bright = stage->brightness(); // Get brightness

// Frame timing
float delta = stage->deltaTime(); // Time since last frame
float fps = stage->frameRate();   // Current frame rate

// Scene information
const auto& info = stage->currentScene(); // Current scene info
size_t count = stage->sceneCount();      // Number of scenes
```

## Best Practices

1. **Resource Management**:
   - Use smart pointers for components
   - Let Stage manage scene lifecycle
   - Clean up resources in scene destructors

2. **Performance**:
   - Maintain consistent frame rate
   - Minimize allocations in update loop
   - Use platform-specific optimizations

3. **Error Handling**:
   - Check scene existence before use
   - Validate transitions
   - Provide fallback scenes

4. **Scene Organization**:
   - Register scenes at startup
   - Use meaningful names and descriptions
   - Group related scenes together

See [Scene Documentation](Scenes.md) for details on implementing scenes.
