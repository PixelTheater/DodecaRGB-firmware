# Creating Animations (Scenes)

This guide walks you through creating a basic animation (called a "Scene") using the PixelTheater library.

## 1. Prerequisites

*   Familiarity with C++.
*   PlatformIO environment set up for the project.
*   A generated Model header file (e.g., `models/MyModel/model.h`) defining your LED geometry.

## 2. Create the Scene File

Create a new header file for your scene, typically within the `src/scenes/` directory (e.g., `src/scenes/my_scene/my_scene.h`).

## 3. Basic Scene Structure

Start with the basic class structure, inheriting from `PixelTheater::Scene`:

```cpp
#pragma once

#include "PixelTheater.h" // Include the main library header

namespace Scenes {

class MyScene : public PixelTheater::Scene {
public:
    // Constructor (usually default is fine)
    MyScene() = default;
    
    // Required lifecycle methods
    void setup() override {
        // Initialization code goes here
        set_name("My First Scene"); // Set basic metadata
    }

    void tick() override {
        // Animation logic goes here (runs every frame)
        Scene::tick(); // Recommended to call base tick
    }
};

} // namespace Scenes
```

*   Include `PixelTheater.h`.
*   Use a namespace (e.g., `Scenes`).
*   Inherit publicly from `PixelTheater::Scene`.
*   Override the pure virtual `setup()` method.
*   Override the `tick()` method for animation logic.

### Including Headers

Simply include the main library header `#include "PixelTheater.h"`. This provides access to the `Scene` base class, core types (`CRGB`, `Point`, `Face`), the `IModel` interface, utility functions (`map`, `nblend`, etc.), and constants (`Constants::PT_PI`).

Consider adding `using namespace PixelTheater;` for convenience.
To easily access constants like `PT_PI`, `PT_TWO_PI`, etc., add:
`using namespace PixelTheater::Constants;`

## 4. Defining Parameters

Make your scene configurable by adding parameters in `setup()` using the `param()` helper:

```cpp
void setup() override {
    set_name("My Scene Name");
    set_description("A description of what this scene does");
    
    // Simple float parameter (0.0 to 1.0)
    param("speed", "ratio", 0.5f, "clamp", "Animation speed");
    
    // Integer parameter with range [1, 10]
    param("count", "count", 1, 10, 5, "", "Number of items");
    
    // Boolean switch
    param("enabled", "switch", true, "", "Enable the effect");
}
```

See `Parameters.md` for all available parameter types and flags.

## 5. Implementing Animation Logic (`tick()`)

Use the `tick()` method to update the animation state and set LED colors. Access LEDs, geometry, parameters, and utilities via the `Scene` base class helpers:

```cpp
void tick() override {
    Scene::tick(); // Track frame count

    // Get parameter values
    float speed = settings["speed"];
    int count = settings["count"];
    bool enabled = settings["enabled"];

    if (!enabled) {
        // Optionally clear LEDs if disabled
        for(size_t i=0; i < ledCount(); ++i) leds[i] = PixelTheater::CRGB::Black;
        return; // Skip rest of animation
    }

    // Get time
    uint32_t time_ms = millis();
    
    // Fade existing colors slightly
    for(size_t i=0; i < ledCount(); ++i) {
        leds[i].fadeToBlackBy(10); 
    }

    // Animate based on parameters and geometry
    for (size_t i = 0; i < ledCount(); ++i) {
        const auto& p = model().point(i); // Get point for this LED
        
        // Example: pulse brightness based on Z coordinate and time
        float phase = (p.z() * 0.1f) + (time_ms / 1000.0f * speed * PixelTheater::Constants::PT_TWO_PI);
        uint8_t brightness = PixelTheater::sin8(static_cast<uint8_t>(phase * 255 / PixelTheater::Constants::PT_TWO_PI));
        
        // Set LED color (only if brightness > 0)
        if (brightness > 10) { // Avoid setting near-black colors constantly
           leds[i] = PixelTheater::CHSV(160, 255, brightness); // Blue/Green hue
        }
    }
}
```

**Key Helpers Used:**
*   `settings["param_name"]`: Access parameter values.
*   `ledCount()`: Get the number of LEDs.
*   `leds[i]`: Access individual LEDs (returns `PixelTheater::CRGB&`).
*   `model().point(i)`: Get the 3D point (`PixelTheater::Point`) for LED `i`.
*   `millis()`: Get current time.
*   `PixelTheater::sin8()`: Fast trig function.
*   `PixelTheater::CHSV`: Color struct.
*   `PT_PI`, `PT_TWO_PI`: Constants (available via `using namespace PixelTheater::Constants;`).

## 6. Adding the Scene to the Show

In your main application (`src/main.cpp`), include your scene's header and add it to the `Theater` during setup:

```cpp
// In src/main.cpp
#include "PixelTheater.h"
#include "models/MyModel/model.h" // Your model
#include "scenes/my_scene/my_scene.h" // Your new scene

PixelTheater::Theater theater;

void setup() {
    // ... hardware setup ...
    
    theater.useFastLEDPlatform<PixelTheater::Models::MyModel>(...); // Initialize Theater
    
    theater.addScene<Scenes::MyScene>(); // Add your scene
    // Add other scenes...
    
    theater.start(); // Start the show
}

void loop() {
    theater.update(); // Run the current scene
    // ... handle input ...
}
```

Now, when you build and run, your scene will be part of the sequence managed by the `Theater`!

Refer to `Scenes.md` for a more detailed API reference. 