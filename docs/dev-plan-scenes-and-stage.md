# PixelTheater Development Plan

Reference docs:
‼️ @model.md : Model structure and data format documentation
‼️ @scene.md : Scene class documentation

## Refactoring for FastLED and Native Compatible Environments

The primary goal of this part of the project is to support both native (test, simulator) and FastLED (microcontroller hardware) environments while maintaining clean abstractions and testability. 

## Current Status (Updated March 2024)

We are working on final integration of scenes and stage, the model and the platform. We aim to build out the right platform layers and test them.

In the end we will have:  
- fastled/hardware env, src/main.cpp and this is the "firmware"  
- native simulator, no hardware fastled, opengl instead (future feature)  
- the test environment, also in native environment, no fastled, no opengl, simulated scenes and models, no main.cpp or app, only doctest and fixtures and test results.

All environments will use the PixelTheater library.

To setup and use this library, there are several dependencies.
- select a platform (native, fastled, teensy)
- choose a model to run on, which defines the led layout and geometry
- include the scenes you want to use
- setup the stage with the model and led platform

For the firmware environment running on the teensy and real hardware, all of this comes together to in main.cpp:

- First the user needs to load a model. Models are generated and will provide the led count and basic info.
- Then they setup FastLed with the right pins and wiring they prefer (this is specific to the firmware).
- They also need to include one or more scenes.
- The stage is initialized with the model and led platform (fastled), they can add scenes to it and run them.

**Note:** `src/main.cpp` is strictly for building the firmware for the teensy microcontroller. We can think of the files in `/src` and `/include` as our firmware app for Teensy41 environment. It will never run on native.

In the native test environment, we don't have a main.cpp or application, we just have test cases. Our models are fixtures, located in `tests/fixtures/models`. Our led driver is just a mock. Our scenes are just test cases.

In the (future) native simulator environment, we will have something different. There will be a C++ application that uses the PixelTheater library with the same model and scenes. However instead of fastled, it will load a virtual led driver and update a 3d scene in an OpenGL window. This driver will be a bridge between the stage and the opengl window, allowing a user to interact with the virtual model on-screen.

> We are not planning to work on the simulator yet, but we should keep it in mind for the future.

*All scene code should work the same in every platform* without the need for platform specific configuration. *All models are compatible with all scenes and all platforms*, they just tell us about the geometry and led counts and layout.

So in this way, we need our hardware dependencies to end at the stage level, so that a scene works exactly the same wether using fastled in firmware or native (test or simulated).

## Implementation Plan

### Current Status (Updated)

1. Basic Operations ✅
   - [x] Test LED array access patterns
   - [x] Verify zero-copy performance
   - [x] Test brightness control
   - [x] Platform integration tests

2. Face/Model Integration ⏳
   - [x] Test Face LED group operations
   - [x] Verify Model LED indexing
   - [x] Check array bounds handling
   - [ ] Fix remaining test constructor issues

3. Core Color System Native Support ✅
   - [x] Review current color operations
   - [x] Ensure color math works in native env
   - [x] Add comprehensive native color tests
   - [x] Verify color operations in isolation

4. Face/Model Native Updates ⏳
   - [x] Update Face class for platform abstraction
   - [ ] Verify Model works with new Stage
   - [x] Add Face/Model native tests
   - [x] Test geometric operations

5. FastLED Platform Implementation ⏳
   - [x] Implement FastLEDPlatform
   - [ ] Add FastLED-specific optimizations
   - [ ] Create hardware test suite
   - [ ] Check timeprovider, mathprovider,constants, etc
   - [ ] Test on Teensy environment

### Next Steps

A. Fix Test Infrastructure
- Fix remaining test constructor issues
- Update test fixtures for consistency
- Add more helper methods as needed

B. Stage Implementation
- Design Stage interface
- Implement basic Stage functionality
- Add Scene management
- Test scene transitions

C. Hardware Testing
- Set up hardware test environment
- Create hardware-specific tests
- Verify FastLED integration
- Test performance on target hardware

### Testing Strategy

1. Native Environment (First Priority)
   - [x] Core color operations
   - [x] Model/Face relationships
   - [x] Platform abstraction
   - [ ] Stage management

2. Hardware Environment (Next Phase)
   - [ ] Basic LED operations
   - [ ] Color accuracy
   - [ ] Performance benchmarks
   - [ ] Memory usage

### Validation Requirements

- All tests pass in native environment
- Color operations match FastLED behavior
- Face boundaries work correctly
- Scene transitions are smooth
- Memory usage within budget

## Architecture Overview

Key Design Points:

- Scene code is identical between platforms
- LED array memory is shared (no copying)
- Platform differences isolated to Platform layer
- Hardware setup remains in firmware
- Native environment provides simulation
- Model geometry works the same everywhere
- Clean, intuitive API hides complexity

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

**Native Environment (simulator)**

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
