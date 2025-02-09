---
category: Development
generated: 2025-02-09 15:22
version: 2.8.1
---

# [8] Build System

The build system is a collection of python scripts in [/utils](../../util/) that are used to generate the C++ code from the YAML scene definitions.

## [8.1] Code Generation

The build system converts YAML parameter definitions into C++ code that the Scene and Settings classes use at runtime. Details about the python environment and how to run the scripts are in [utils/README.md](../../util/README.md).

## Parameter Code Generation

For each scene's YAML file, the generator creates a C++ `_params.h` header file with parameter definitions, which looks like this:

```cpp
// scenes/fireworks/_params.h
// Auto-generated from scenes/fireworks/fireworks.yaml
#pragma once
#include "PixelTheater/parameter.h"

namespace PixelTheater {

// Option arrays for select parameters
static constexpr const char* const pattern_options[] = {
    "sphere", "fountain", "cascade", nullptr
};

// Parameter definitions
constexpr ParamDef FIREWORKS_PARAMS[] = {
    PARAM_SWITCH("sparkle", true, "Enable sparkle effect"),
    PARAM_COUNT("num_particles", 10, 1000, 100, Flags::NONE, "Number of particles"),
    PARAM_RATIO("speed", 0.5f, Flags::CLAMP, "Animation speed multiplier"),
    PARAM_SELECT("pattern", 0, pattern_options, "Animation pattern")
};

} // namespace PixelTheater
```

When the scene is compiled, the `_params.h` file is included and the `FIREWORKS_PARAMS` array is used to initialize the scene's parameters. It is then available for use in via the `settings[]` array.

```cpp
// getters
int num = settings["num_particles"];
float speed = settings["speed"] = 0.5f;
// setters
settings["sparkle"] = true;
```

### scene_generator.py: YAML to C++

The build process generates C++ code from YAML that defines parameters, presets, and props for a scene. The following shows how the files are organized and generated:

```text
scenes/
├── fireworks/
│   ├── fireworks.yaml       # Scene definition
│   ├── fireworks.cpp        # Scene implementation (setup, tick, status, reset)
│   ├── _params.h            # _Generated_ parameter structs
│   ├── README.md            # User documentation
│   └── props/               # Scene-specific props
│       └── sparks.pal.json
└── space/
    └── space.yaml
    └── space.cpp
```

### [8.2] LED Coordinates

LED positions are calculated from PCB pick-and-place data:

1. Each PCB face has 104 LEDs in a pentagon shape
2. 12 faces form a dodecahedron ~13cm in diameter
3. Coordinates are generated in several formats:
   - Cartesian (x,y,z)
   - Spherical (radius, theta, phi)
   - Face/index pairs

![PCB Pick-and-Place](../../images/leds-3d-space.png)

The build process:

1. Reads pick-and-place CSV files
2. Applies face rotations and transformations
3. Generates C++ point data
4. Creates neighbor lookup tables
5. Generates `points.h` header file

The generated `points.h` defines an array of `LED_Point` objects that represent the physical LEDs in the dodecahedron (12 pentagon faces × 104 LEDs per face = 1,248 total LEDs).

Here's an example of how the generated points array might look:

```cpp
// points.cpp

LED_Point(23, -64.502, -53.968, 262.000, 24, 0, {{.led_number = 42, .distance = 28.726}, {.led_number = 10, .distance = 29.271}, {.led_number = 24, .distance = 31.916}, {.led_number = 22, .distance = 32.298}, {.led_number = 43, .distance = 37.444}, {.led_number = 41, .distance = 42.067}, {.led_number = 9, .distance = 46.032}}),
LED_Point(24, -80.056, -26.098, 262.000, 25, 0, {{.led_number = 43, .distance = 31.889}, {.led_number = 23, .distance = 31.916}, {.led_number = 44, .distance = 32.458}, {.led_number = 25, .distance = 32.605}, {.led_number = 10, .distance = 35.475}, {.led_number = 11, .distance = 35.680}, {.led_number = 42, .distance = 49.358}}),
LED_Point(25, -84.746, 6.168, 262.000, 26, 0, {{.led_number = 45, .distance = 28.557}, {.led_number = 11, .distance = 30.515}, {.led_number = 24, .distance = 32.605}, {.led_number = 26, .distance = 33.386}, {.led_number = 44, .distance = 36.707}, {.led_number = 46, .distance = 42.276}, {.led_number = 12, .distance = 47.436}}),

```

### [8.3] Environments

- **teensy41**: Builds firmware with Arduino framework, generates LED points
- **native**: Builds test suite with doctest, generates LED points and test fixtures