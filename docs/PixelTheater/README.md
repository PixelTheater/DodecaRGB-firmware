---
author: Jeremy Seitz - somebox.com
generated: 2025-02-03 01:26
project: DodecaRGB Firmware
repository: https://github.com/somebox/DodecaRGB-firmware
title: PixelTheater Animation System
version: 2.8.0
---

<div style="display: flex; justify-content: space-between; align-items: center;">
            <div>
                <p style="font-size: 1.0em; color: #888;">Documentation for <a href="https://github.com/somebox/DodecaRGB-firmware">DodecaRGB Firmware</a></p>
            </div>
            <div style="text-align: right; font-size: 0.7em; color: #888;">
                <p>Version 2.8.0<br/>
                Generated: 2025-02-03 01:26</p>
            </div>
          </div>

# PixelTheater Animation System

## [1] Overview

The animation system provides a type-safe, flexible framework for creating LED animations on three-dimensional objects. Whether you're building a dodecahedron, sphere, cube, or any other LED-covered shape, this library makes it easy to:

- Create modular, reusable animations (scenes)
- Define configurable parameters for each animation
- Switch between animations smoothly
- Define animation parameters and presets
- Integrate with sensors and user input
- Debug and monitor animation performance

### [2] Architecture and Class Structure

```text
┌──────────┐                          
│ Director │                          
└┬─────────┘                          
 │  ┌────────┐                        
 ├─▶│ Show   │                        
 │  └┬───────┘                        
 │   │  ┌────────┐┌────────┐┌────────┐
 │   └─▶│Scene 1 ││Scene 2 ││Scene N │
 │      └┬───────┘└────────┘└────────┘
 │       │  ┌─────────────┐  ┌───────┐  ┌───────┐
 │       ├─▶│Settings     │◀─┤Presets│◀─┤Actors │
 │       │  └─────────────┘  └───────┘  └───────┘
 │       │  ┌────────┐   ┌───────────┐
 │       └─▶│Controls│◀──┤Controllers│
 │          └────────┘   └───────────┘
┌┴───────┐    ╔════════════════╗      
│ Stage  │───▶║ current scene  ║      
└┬───────┘    ╚════════════════╝      
 │  ┌────────────────┐                   
 └─▶│ VenueDevice    │                   
    └┬───────────────┘                   
     │  ┌────────────┐                
     ├─▶│ LEDSurface │                
     │  └────────────┘                
     │  ┌────────────┐                
     └─▶│ HWDevices  │                
        └────────────┘                             
```

### [2.1] Key Concepts

- **Stage**: The virtual spherical display where the scene is rendered
- **Director**: Manages the animation system, including scene selection and transitions
- **Show**: A list of scenes to play
- **Scene**: A single animation, including its parameters and behavior
- **Controls**: A set of controls that can be used to interact with the scene
- **Presets**: A snapshot of settings for the controls, props and scenes
- **Props**: Chunks of data like color palettes, bitmaps, geometry used by the scene
- **Actors**: Animation objects (classes) used in a scene
- **Controllers**: An external interface to drive scene controls in real time
- **Settings**: The configuration of controls, props and presets


The Director manages scene transitions and ensures proper lifecycle method calls.

## [5] Directing Scenes

The Director is responsible for selecting and transitioning between scenes. It can place animations on the stage (run them), and manage playlists and activate presets. The Director puts on the show.

## [6] Props System

Props are binary assets (palettes, bitmaps) that can be:

- Global: defined in props.yaml

## [13] Advanced Configuration

### Build Process

The build system processes scene YAML files to generate C++ code:

1. During build, `generate_scenes.py` is called for each scene YAML file
2. The generator creates a header file in the same directory as the scene YAML file named `_params.h`.
3. At compile time, the scene automatically includes `_params.h` to get the parameter definitions.

The header file uses macros to define the parameters with:

- Type mapping (signed_ratio → float with -1..1 range)
- Flag conversion (clamp → ParamFlag::Clamp)
- Range validation
- Test fixture generation
- Descriptions preserved

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

### Testing

Tests are organized into two environments:

```bash
# Run native tests
pio test -e native

# Run specific test file
pio test -e native -f test_parameters
```

Test fixtures are generated from scene YAML files to enable testing with real scene configurations:

```cpp
// Generated fixture provides scene parameters
struct SpaceSceneFixture {
    PixelTheater::SpaceSceneParameters params;
};

// Use fixture in tests
TEST_CASE_FIXTURE(SpaceSceneFixture, "Scene parameters work") {
    // Test semantic type ranges
    CHECK(params.speed.get() == 0.5f);          // signed_ratio [-1.0, 1.0]
    CHECK(params.brightness.set(0.8f) == true); // ratio [0.0, 1.0]
    
    // Test select with mapped values
    CHECK(params.direction.get() == -1);        // "reverse" maps to -1
}
```

The doctest framework and PlatformIO's toolchains are used for testing. The native test environment only tests the library code, not the hardware. That means the arduino framework and FastLED are mocked out.

```cpp
// Generated palette_data.h
namespace PixelTheater {
    // Each palette is a separate const struct
    constexpr struct {
        const uint8_t data[12] = {
            0,   255, 0,   0,    // red
            128, 0,   255, 0,    // green
            255, 0,   0,   255   // blue
        };
    } PALETTE_RAINBOW;

    // Simple lookup returns pointer to palette data
    const uint8_t* get_palette(const char* name);
} 
```