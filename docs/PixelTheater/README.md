---
author: Jeremy Seitz - somebox.com
generated: 2025-02-04 07:23
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
                Generated: 2025-02-04 07:23</p>
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

# Run python tests (from root of repo)
python -m util.tests.run_tests
```

The [fireworks.yaml](utils/test/fixtures/fireworks.yaml) file is used to test fixture code generation, and the generated file is written to the `test/fixtures/` directory. Running the python tests will generate the files needed for the C++ tests.

The C++ doctest framework is used for testing. PlatformIO's toolchains are used for the C++ tests. The native test environment only tests the library code, not the hardware. That means the arduino framework and FastLED are mocked out.

The python tests are run with the `run_tests.py` script, which also formats the output. This runs all the tests in the `util/tests` directory. Both doctest and testunit are used for the python tests.