# PixelTheater Development Plan

Reference docs:
‼️ @model.md : Model structure and data format documentation
‼️ @scene.md : Scene class documentation

## Current Status (Updated March 2024)

### Refactoring for FastLED and Native Compatible Environments

The primary goal is to support both native (test/development) and FastLED (hardware) environments while maintaining clean abstractions and testability.

#### Architecture Overview

1. Core Components and Flow
```cpp
// Component hierarchy and data flow
Scene (Animation Logic)
  -> Stage (Platform Abstraction)
     -> Platform (Hardware Interface)
        -> LED Array (Shared Memory)
```

2. Component Responsibilities

**Scene Layer** (Animation Logic)
- Defines animation behavior and parameters
- Accesses LEDs through Stage interface
- Uses Model for geometry/mapping
- Platform agnostic - same code runs everywhere
```cpp
class MyScene : public Scene {
    void tick() override {
        auto* leds = _stage.leds();  // Get LED array
        auto face = _stage.model()->getFace(0);  // Get geometry
        // Update animation...
    }
};
```

**Stage Layer** (Platform Abstraction)
- Owns Platform implementation
- Provides LED access to Scenes
- Manages Model for geometry
- Handles scene lifecycle
```cpp
class Stage {
    std::unique_ptr<Platform> _platform;
    std::unique_ptr<Model> _model;
    
    CRGB* leds() { return _platform->getLEDs(); }
    void update() {
        current_scene->tick();
        _platform->show();
    }
};
```

**Platform Layer** (Hardware Interface)
- Owns LED array memory
- Handles hardware-specific updates
- Two implementations:
  1. FastLEDPlatform for hardware
  2. NativePlatform for simulation
```cpp
class FastLEDPlatform : public Platform {
    CRGB* getLEDs() override { return _leds; }
    void show() override { FastLED.show(); }
};

class NativePlatform : public Platform {
    CRGB* getLEDs() override { return _leds; }
    void show() override { updateOpenGLDisplay(); }
};
```

3. Integration Points

**Firmware Environment** (`/src/main.cpp`)
```cpp
// Hardware-specific setup
CRGB leds[NUM_LEDS];
FastLED.addLeds<WS2812B, PIN, GRB>(leds, NUM_LEDS);

// Simple platform and scene setup
stage.useFastLED(leds);
auto* space = stage.addScene<SpaceScene>();
auto* fire = stage.addScene<FireScene>();

void loop() {
    stage.update();
}
```

**Native Environment**
```cpp
// Create OpenGL window
auto window = createOpenGLWindow();

// Same simple API as firmware
stage.useNative(window);
stage.addScene<SpaceScene>();

while (running) {
    stage.update();
    window.swapBuffers();
}
```

4. Implementation Details

**Platform Interface**
```cpp
class Platform {
    // Core LED array management
    virtual CRGB* getLEDs() = 0;               // Returns LED array pointer for zero-copy access
    virtual size_t getNumLEDs() const = 0;     // Returns total number of LEDs
    
    // Hardware control operations
    virtual void show() = 0;                   // Triggers LED update (FastLED.show())
    virtual void setBrightness(uint8_t) = 0;   // Sets global brightness
    virtual void clear() = 0;                  // Clears all LEDs to black
    
    // Performance settings
    virtual void setMaxRefreshRate(uint8_t fps) = 0;  // Caps update rate
    virtual void setDither(uint8_t) = 0;             // Controls color dithering
};
```

**Stage Implementation**
```cpp
class Stage {
public:
    // Friendly platform initialization
    void useFastLED(CRGB* leds);
    void useNative(OpenGLWindow& window);
    
    // Type-safe scene creation
    template<typename SceneType, typename... Args>
    SceneType* addScene(Args&&... args);
    
    // Core functionality
    CRGB* leds();
    void update();
    void setBrightness(uint8_t);
    
private:
    // Internal ownership management
    void setPlatform(std::unique_ptr<Platform>);
    std::vector<std::unique_ptr<Scene>> _scenes;
    std::unique_ptr<Platform> _platform;
    std::unique_ptr<Model> _model;
};
```

5. API Design Principles

The Stage API is designed to be intuitive while maintaining strong ownership semantics internally:

**Platform Setup**
```cpp
class Stage {
public:
    // Friendly platform initialization
    void useFastLED(CRGB* leds);
    void useNative(OpenGLWindow& window);
    
    // Type-safe scene creation
    template<typename SceneType, typename... Args>
    SceneType* addScene(Args&&... args);
    
private:
    // Internal ownership management
    void setPlatform(std::unique_ptr<Platform>);
    std::vector<std::unique_ptr<Scene>> _scenes;
};
```

Key API Features:
- Simple, intuitive method names
- Type-safe scene creation
- Hidden memory management
- Platform-specific setup helpers
- Returns raw pointers for scene configuration

Benefits:
- Clean, readable main.cpp
- Same API works for both platforms
- Strong ownership maintained internally
- Type safety at compile time
- Easy scene management

5. Key Design Points

- Scene code is identical between platforms
- LED array memory is shared (no copying)
- Platform differences isolated to Platform layer
- Hardware setup remains in firmware
- Native environment provides simulation
- Model geometry works the same everywhere
- Clean, intuitive API hides complexity

#### Implementation Plan

1. Platform Interface & Native Implementation (Priority: HIGH)
```cpp
class Platform {
    // Core LED array management
    virtual CRGB* getLEDs() = 0;               // Returns LED array pointer for zero-copy access
    virtual size_t getNumLEDs() const = 0;     // Returns total number of LEDs
    
    // Hardware control operations
    virtual void show() = 0;                   // Triggers LED update (FastLED.show())
    virtual void setBrightness(uint8_t) = 0;   // Sets global brightness
    virtual void clear() = 0;                  // Clears all LEDs to black
    
    // Performance settings
    virtual void setMaxRefreshRate(uint8_t fps) = 0;  // Caps update rate
    virtual void setDither(uint8_t) = 0;             // Controls color dithering
};
```

Implementation Steps:
- [ ] Create NativePlatform implementation with array ownership
- [ ] Implement zero-copy LED access for Face/Model use
- [ ] Add array bounds checking in debug builds
- [ ] Create mock FastLED functions for native testing

Validation:
1. Array Management
   - Test array access patterns (direct, Face, Model)
   - Verify zero-copy performance
   - Check bounds handling
2. Platform Operations
   - Test brightness control
   - Verify clear operation
   - Test refresh rate settings
3. Integration Tests
   - Test Face LED group operations
   - Verify Model LED indexing
   - Test animation frame updates

2. Stage Basic Integration (Priority: HIGH)
```cpp
class Stage {
    std::unique_ptr<Platform> _platform;
    
    // Platform-agnostic interface
    void show() { _platform->show(); }
    void setBrightness(uint8_t b) { _platform->setBrightness(b); }
    
    // LED array access for Face/Model use
    CRGB* leds() { return _platform->getLEDs(); }
    size_t numLeds() const { return _platform->getNumLEDs(); }
    
    // Scene management (to be implemented)
    void update();
    void addScene(Scene* scene);
    void transitionTo(Scene* scene);
};
```

Implementation Steps:
- [ ] Update Stage to use Platform abstraction
- [ ] Implement basic LED management for native
- [ ] Add array access patterns for Face/Model
- [ ] Create Stage unit tests
- [ ] Add Scene management interface

Validation:
1. Basic Operations
   - Test LED array access patterns
   - Verify zero-copy performance
   - Test brightness control
2. Face/Model Integration
   - Test Face LED group operations
   - Verify Model LED indexing
   - Check array bounds handling
3. Scene Management
   - Test scene transitions
   - Verify update cycle
   - Test animation frame timing

3. Core Color System Native Support (Priority: HIGH)
```cpp
namespace PixelTheater {
  class CRGB {
    // Focus on native implementation first
    // Keep FastLED compatibility in mind
  };
}
```
- [ ] Review current color operations
- [ ] Ensure color math works in native env
- [ ] Add comprehensive native color tests
- [ ] Verify color operations in isolation
Validation: All color operations work correctly in native environment

4. Face/Model Native Updates (Priority: MEDIUM)
- [ ] Update Face class for platform abstraction
- [ ] Verify Model works with new Stage
- [ ] Add Face/Model native tests
- [ ] Test geometric operations
Validation: Face and Model classes work in native environment

5. FastLED Platform Implementation (Priority: HIGH)
```cpp
class FastLEDPlatform : public Platform {
    void show() override { FastLED.show(); }
    // ... other implementations
};
```
- [ ] Implement FastLEDPlatform
- [ ] Add FastLED-specific optimizations
- [ ] Create hardware test suite
- [ ] Test on Teensy environment
Validation: Basic operations work on hardware

6. Platform-Specific Features (Priority: MEDIUM)
- [ ] Add TimeProvider integration
- [ ] Implement hardware-specific features
- [ ] Add performance optimizations
- [ ] Test on both platforms
Validation: All features work on both platforms

7. Advanced Features & Optimization (Priority: LOW)
- [ ] Add SIMD operations for FastLED
- [ ] Optimize color operations
- [ ] Add hardware-specific improvements
- [ ] Benchmark both platforms
Validation: Performance meets requirements on both platforms

#### Testing Strategy

1. Native Environment (First Priority)
- Unit Tests
  - Platform interface implementation
  - LED array access patterns
  - Color operations and math
  - Face/Model LED group operations
  - Scene management and transitions
  
- Integration Tests
  - Face-to-Model LED indexing
  - Animation frame updates
  - Scene transitions and timing
  - Memory usage patterns
  
- Performance Tests
  - Array access benchmarks
  - Color operation speed
  - Animation frame rates
  - Memory allocation patterns

2. Hardware Environment (Second Priority)
- Basic Functionality
  - LED array initialization
  - FastLED integration points
  - Hardware timing verification
  - Power usage monitoring
  
- Feature Validation
  - Color accuracy on hardware
  - Brightness control
  - Refresh rate limits
  - Temperature monitoring
  
- Performance Validation
  - Frame rate stability
  - Color update latency
  - Power consumption patterns
  - Memory usage on device

#### Migration Strategy

1. Native Environment First
- [ ] Implement Platform interface with native array
- [ ] Port existing color operations
- [ ] Add Face/Model LED management
- [ ] Create comprehensive test suite
- [ ] Document native behavior patterns

2. Hardware Integration
- [ ] Create FastLEDPlatform implementation
- [ ] Test zero-copy array access
- [ ] Verify Face/Model operations
- [ ] Add hardware-specific features
- [ ] Document hardware differences

3. Final Integration
- [ ] Test both environments in parallel
- [ ] Measure performance metrics
- [ ] Update documentation
- [ ] Create migration guides
- [ ] Release testing tools

#### Timeline (Revised)

Week 1:
- Implement Platform interface
- Create NativePlatform
- Basic Stage integration
- Native environment tests

Week 2:
- Complete native color system
- Face/Model native updates
- Integration testing
- Native environment validation

Week 3:
- Begin FastLED implementation
- Port to Teensy environment
- Hardware-specific features
- Basic hardware testing

Week 4:
- Complete hardware integration
- Performance optimization
- Documentation updates
- Final testing both platforms

### Remaining Features ❌

1. Scene System
- Scene class implementation
- Animation framework
- Transition effects
- Common pattern library

2. Stage Implementation
- Scene management
- Animation timing
- Global effects
- Hardware synchronization

## Next Steps (Prioritized)

### A. Complete Spatial Query System
Priority: HIGH
Estimated time: 2-3 days

1. Core Implementation
```cpp
// Proposed API
class SpatialIndex {
    std::vector<size_t> findNearby(const Point& p, float radius);
    std::vector<size_t> findNearbyFast(size_t led_index, float radius);
    float distanceBetween(size_t led1, size_t led2);
};
```

2. Tasks
- [ ] Implement findNearby() using existing neighbor data
- [ ] Add spatial indexing for performance
- [ ] Create distance-based helper functions
- [ ] Add unit tests
- [ ] Benchmark performance

### B. Scene System Implementation
Priority: MEDIUM
Estimated time: 1 week

1. Core Components
```cpp
class Scene {
    virtual void setup();
    virtual void update(uint32_t ms);
    virtual void enter();
    virtual void exit();
    
    // Animation helpers
    void crossfade(const Scene& other, uint16_t duration_ms);
    void transition(TransitionType type, uint16_t duration_ms);
};
```

2. Tasks
- [ ] Define Scene lifecycle
- [ ] Implement basic animation framework
- [ ] Create common pattern library
- [ ] Add transition effects
- [ ] Document usage patterns

### C. Stage Implementation
Priority: MEDIUM
Estimated time: 3-4 days

1. Core Features
```cpp
class Stage {
    void addScene(Scene* scene);
    void transitionTo(Scene* scene, uint16_t duration_ms);
    void update();
    void setBrightness(uint8_t brightness);
};
```

2. Tasks
- [ ] Complete Stage class implementation
- [ ] Add scene management
- [ ] Implement timing system
- [ ] Add global effects (brightness, color correction)
- [ ] Create example animations

### D. Performance Optimization
Priority: LOW
Estimated time: Ongoing

1. Focus Areas
- [ ] Profile LED group operations
- [ ] Optimize spatial queries
- [ ] Measure memory usage
- [ ] Add benchmarks

## Timeline

Week 1:
- Complete Spatial Query System
- Start Scene implementation

Week 2:
- Complete Scene implementation
- Start Stage implementation

Week 3:
- Complete Stage implementation
- Begin optimization
- Documentation updates

## Questions and Decisions

1. Scene System Design
- How should scenes handle state persistence?
- What common animation patterns should be provided?
- How to handle resource management?

2. Performance Considerations
- Maximum number of concurrent animations?
- Memory constraints for different platforms?
- Update frequency requirements?

Would you like to:
A. Start implementing the Spatial Query System?
B. Begin Scene system design?
C. Add more detail to any section?
