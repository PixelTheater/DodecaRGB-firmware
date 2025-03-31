# Easier API Refactoring

## Goals

We want to make it much easier and cleaner for users to define scenes and to use the PixelTheater library. We don't want to needlessly expose them to `<template>` syntax and initialization with `std::move` and `std:make_unique` and so forth. Our target audience are makers that are familiar with microcontrollers and FastLED but looking to build more challenging projects.

This refactoring focuses only on:
1. Simplifying the initialization and setup of the library
2. Making it easier to select models and scenes
3. Reducing boilerplate code
4. Providing a cleaner user-facing API

We are **not** changing:
1. The existing parameter system
2. The core Scene, Stage, or Model implementations
3. The platform abstractions

## Implementation Approach

We'll create a facade layer on top of the existing architecture that:

1. Hides template complexity from users via a `Theater` class
2. Uses `SceneType` objects for type-safe scene references without templates
3. Allows creation of multiple instances of the same scene type
4. Maintains the full power of the existing architecture underneath

## Key Components

### SceneType

Scene types are represented by constant objects that provide type safety without templates:

```cpp
// In a central header
struct SceneType {
    const char* id;         // Internal ID for lookup
    const char* name;       // Display name
    const char* description; // Description
};

// In each scene implementation file
namespace Scenes {
    extern const SceneType XYZScanner;
    extern const SceneType WanderingParticles;
    // Other scene types...
}
```

### Theater Class

The Theater class serves as a facade that hides template complexity:

```cpp
class Theater {
public:
    // Initialize with explicit model type
    template<typename ModelDef>
    bool begin(CRGB* leds, size_t numLeds);
    
    // Add a scene with optional custom instance name
    bool addScene(const SceneType& sceneType, const std::string& instanceName = "");
    
    // Set parameter for a scene instance
    template<typename T>
    bool setSceneParameter(const std::string& instanceName, const std::string& paramName, T value);
    
    // Play a specific scene instance
    bool playScene(const std::string& instanceName);
    
    // Update current scene
    void update();
    
    // Other methods...
};
```

## Target Usage - Setup

```cpp
#include <FastLED.h>
#include "PixelTheater.h"

// Include model
#include "models/DodecaRGBv2/model.h"

// Include scenes we want to use
#include "scenes/xyz_scanner/xyz_scanner_scene.h"
#include "scenes/wandering_particles/wandering_particles_scene.h"
#include "scenes/fire/fire_scene.h"

// Define LEDs
#define NUM_LEDS 1248
CRGB leds[NUM_LEDS];

// Create theater
PixelTheater::Theater theater;

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  
  // Initialize FastLED as usual
  FastLED.addLeds<WS2812B, 6, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(128);
  
  // Initialize theater with explicit model type
  theater.begin<PixelTheater::Models::DodecaRGBv2>(leds, NUM_LEDS);
  
  // Add scenes using scene type objects
  theater.addScene(Scenes::XYZScanner);  // Uses "XYZ Scanner" as instance name
  theater.addScene(Scenes::WanderingParticles);  // Uses "Wandering Particles"
  theater.addScene(Scenes::Fire);  // Uses "Fire" as instance name
  
  // Create a second instance of XYZScanner with a custom name
  theater.addScene(Scenes::XYZScanner, "Fast Scanner");
  
  // Configure settings for specific instances
  theater.setSceneParameter("XYZ Scanner", "speed", 0.3f);
  theater.setSceneParameter("Fast Scanner", "speed", 0.8f);
  
  // Play a specific scene instance
  theater.playScene("XYZ Scanner");
  
  // Print all available scene instances
  Serial.println("Available scenes:");
  for (const auto& name : theater.getSceneInstances()) {
    Serial.println("- " + name);
  }
}

void loop() {
  // Update the animation
  theater.update();
  
  // Handle user input for scene changes
  if (digitalRead(BUTTON_PIN) == LOW) {
    // Switch to next scene
    static int currentScene = 0;
    const char* scenes[] = {"XYZ Scanner", "Wandering Particles", "Fire", "Fast Scanner"};
    
    currentScene = (currentScene + 1) % 4;
    theater.playScene(scenes[currentScene]);
    
    Serial.println("Now playing: " + String(scenes[currentScene]));
    delay(200);  // Simple debounce
  }
}
```

## Scene Implementation

Scene implementations remain unchanged, using the existing template-based Scene class:

```cpp
// In xyz_scanner_scene.h
namespace Scenes {

// Scene type descriptor - publicly accessible
extern const SceneType XYZScanner;

template<typename ModelDef>
class XYZScannerScene : public PixelTheater::Scene<ModelDef> {
public:
    using Scene = PixelTheater::Scene<ModelDef>;
    using Scene::Scene;

    void setup() override {
        this->set_name("XYZ Scanner");
        this->set_description("Scans the model with moving planes along X, Y, and Z axes");
        // ... rest of setup ...
    }
    
    void tick() override {
        // ... animation code ...
    }
};

} // namespace Scenes

// In xyz_scanner_scene.cpp
namespace Scenes {
    const SceneType XYZScanner = {
        "xyz_scanner",
        "XYZ Scanner",
        "Scans the model with moving planes along X, Y, and Z axes"
    };
}
```

## Advanced Features

### Creating Multiple Scene Instances

The same scene type can be instantiated multiple times with different settings:

```cpp
// Add multiple instances of the same scene type
theater.addScene(Scenes::XYZScanner);  // Default instance name: "XYZ Scanner"
theater.addScene(Scenes::XYZScanner, "Fast Scanner");
theater.addScene(Scenes::XYZScanner, "Slow Scanner");

// Configure each instance differently
theater.setSceneParameter("XYZ Scanner", "speed", 0.5f);  // Default speed
theater.setSceneParameter("Fast Scanner", "speed", 0.9f);  // Fast
theater.setSceneParameter("Slow Scanner", "speed", 0.2f);  // Slow
```

### Creating a Playlist

You can create and manage a playlist of scenes:

```cpp
// Create a playlist of scenes
std::vector<std::string> playlist = {
    "XYZ Scanner",
    "Wandering Particles",
    "Fire",
    "Fast Scanner"
};

int currentIndex = 0;

// In a button handler or timer:
void nextInPlaylist() {
    currentIndex = (currentIndex + 1) % playlist.size();
    theater.playScene(playlist[currentIndex]);
    Serial.println("Now playing: " + playlist[currentIndex]);
}
```

## Implementation Details

### Theater Implementation

The Theater class internally uses type erasure to hide the template complexity:

1. **Stage Management**: Manages a type-erased Stage instance
2. **Scene Creation**: Creates typed Scene instances based on SceneType
3. **Instance Management**: Tracks scene instances by name
4. **Parameter Access**: Provides access to scene parameters

### Scene Type Registration

Scene types are defined as global constants in each scene's implementation file. No explicit registration is needed beyond including the scene files.

### Model Detection

The Theater class is initialized with an explicit model type, eliminating the need for runtime model detection.

## Benefits

1. **Clean API**: No templates or complex memory management in user code
2. **Type Safety**: SceneType objects provide type checking without templates
3. **Reduced Boilerplate**: Simplified scene creation and management
4. **Multiple Instances**: Support for multiple instances of the same scene type
5. **Existing Architecture**: Preserves the power of the existing implementation
6. **Intuitive Naming**: Uses scene's internal name by default, with option for custom names
