---
category: Development
generated: 2025-02-10 00:32
version: 2.8.2
---

# PixelTheater Animation System

## Overview

The animation system provides a type-safe, flexible framework for creating LED animations on three-dimensional objects. Whether you're building a dodecahedron, sphere, cube, or any other LED-covered shape, this library makes it easy to:

- Create modular, reusable animations (scenes)
- Define configurable parameters for each animation
- Switch between animations smoothly
- Define animation parameters and presets
- Integrate with sensors and user input
- Debug and monitor animation performance

### Architecture and Class Structure

```text
┌──────────┐                                                   
│ Director │                                                   
└┬─────────┘                                                   
 │  ┌────────┐                                                 
 ├─▶│  Show  │ - configure and prepare scenes                  
 │  └┬───────┘                                                 
 │   │  ┌────────┐         ┌───────┐                           
 │   └─▶│ Scene  │     ┌───┤Presets│                           
 │      └┬───────┘     ▼   └───────┘                           
 │       │  ┌───────────┐  ┌──────────┐  - types & ranges      
 │       ├─▶│ Settings  │◀─┤Parameters│  - constants & flags   
 │       │  └───────────┘  └──────────┘  - values & validation 
 │       │  ┌────────┐                                         
 │       └─▶│ Props  │ - palettes and bitmaps                  
 │          └────────┘                                         
┌┴───────┐    ╔════════════════╗                               
│ Stage  │───▶║ current scene  ║                               
└┬───────┘    ╚════════════════╝                               
 │  ┌────────┐              ▲                                  
 ├─▶│ Model  │ geometry     │                                  
 │  └┬───────┘              │                                  
 │   │  ┌────────────┐      │                                  
 │   └─▶│ LEDSurface │linear│                                  
 │      └────────────┘      │                                  
 │   ┌────────────┐   ┌─────┴─────┐                            
 └──▶│  Devices   ├──▶│Controllers│ - sensors, events          
     └────────────┘   └───────────┘                                              
```

### Key Concepts

- **Stage**: The place where animated scenes are played on a model covered in LEDs
  - **Model**: Definition of a geometric shape covered with LEDs, generated from hardware files
  - **LEDSurface**: The configured driver for addressable LEDs of a given model
- **Scene**: A single animation, including its parameters and behavior
  - **Parameters**: The types and ranges that define how a scene is configured
  - **Settings**: The interface and internal state of a scene's parameters
  - **Presets**: A snapshot of settings for a scene
  - **Props**: Chunks of data like color palettes, bitmaps, or datasets used by the scene
  - **Actors**: Animation objects (classes) used in a scene
- **Show**: A sequenced list of scenes to play
  - **Director**: Manages the performance: scene selection, transitions, behavior
- **Controls**: Hardware events or sensors that enable interaction
  - **Controllers**: An control mapped to a parameter of a scene

The Director manages scene transitions and ensures proper lifecycle method calls.

## Directing Scenes

The Director is responsible for selecting and transitioning between scenes. It can place animations on the stage (run them), and manage playlists and activate presets. The Director puts on the show.

## Props System

Props are binary assets (palettes, bitmaps) that can be used in scenes.

## Parameter System

Parameters allow scenes to be configured at runtime. They are defined in the scene's `setup()` method:

```cpp
void setup() override {
    // Float parameter with range [0.0, 1.0]
    param("speed", "ratio", 0.5f, "clamp", "Controls animation speed");
    
    // Integer parameter with range [0, 100]
    param("count", "count", 0, 100, 50, "", "Number of particles");
    
    // Boolean parameter
    param("trails", "switch", true, "", "Enable motion trails");
}
```

Parameters can be accessed using the settings object:

```cpp
float speed = settings["speed"];
int count = settings["count"];
bool trails = settings["trails"];
```

The parameter system also supports schema generation for UI rendering and documentation:

```cpp
// Get parameter schema as JSON
auto schema = scene.parameter_schema().to_json();
```

## [13] Advanced Configuration

### Build Process

The build system compiles scenes and models into the firmware:

1. Each scene is compiled as a separate class
2. Models are generated from their definitions into C++ header files
3. The firmware links everything together at compile time

### Customization

PixelTheater can be customized in several ways:

1. Creating new scenes
2. Defining new models
3. Extending the core library with new features

See the individual documentation pages for more details on each aspect of the system.

### [14] Palettes

Palettes define color schemes that can be used in animations. See [Palettes.md](Palettes.md)
for detailed documentation on:

- Available built-in palettes
- Creating custom palettes
- Using palettes in animations
- Memory and performance considerations

### Bitmaps

Bitmap resources can be used for textures, masks, or lookup tables:

- Supported formats: 8-bit grayscale, 24-bit RGB
- Images are converted to binary data at build time
- Access via resource manager to save RAM
- Consider memory limits when using large images
