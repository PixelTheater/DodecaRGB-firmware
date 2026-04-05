# DodecaRGB V2

An open-source LED animation platform for a hand-held dodecahedron with 1620 individually addressable RGB LEDs across 12 pentagon-shaped PCBs. Animations are resolution-independent and run on a Teensy 4.1 microcontroller at up to 80fps.

[![DodecaRGB v2 Teaser Video](images/yt-preview-thumb.png)](https://www.youtube.com/watch?v=RErgt5O7D7U)

![fits in the hand](<images/v2-juggle.gif>)

## Quick Links

| I want to... | Go to |
|---|---|
| Build the hardware | [The Hardware](#the-hardware) |
| Flash the firmware | [Getting Started](#getting-started) |
| Configure my model | [Configuration](#configuration) |
| Change which animations play | [Animations](#animations) |
| Create a new animation | [Creating Animations Guide](docs/guides/creating_animations.md) |
| Set up a dev environment | [Development Guide](docs/guides/development.md) |
| Use the web simulator | [Web Simulator Guide](docs/guides/web-simulator.md) |
| Learn the PixelTheater API | [PixelTheater Docs](docs/PixelTheater/README.md) |

## What Is It?

DodecaRGB V2 is a modular, high-resolution programmable blinky gadget made for the hacker community, especially for coders that want to do spherical animations. Typically, this is quite hard - you either need to make a spinning POV globe, or wire up custom solutions - not to mention power and performance challenges.

This project aims to create a standard platform for resolution-independent animations based on spherical models, using a library called [PixelTheater](docs/PixelTheater/README.md). DodecaRGB V2 is the second iteration, and serves as a reference implementation.

The firmware, 3D models and tooling are open source and free to use and modify. The hardware PCBs are planned as a kit, which will include most of the parts needed to 3D print and assemble your own.

> The implementation is open source and could be applied to other models with different shapes and LED layouts.

## The Hardware

**Note: These are prototypes, and the final hardware will be a bit more *refined and tested*.**

- 12 pentagon-shaped PCBs, each with 135 WS2812B LEDs (SMD 1615 package)
- 1620 LEDs total, wired in series across all 12 sides
- A Teensy 4.1 microcontroller controls everything
- 4 parallel FastLED output channels (GPIO pins 19, 18, 14, 15) with 405 LEDs per channel
- Level shifters, power regulation, battery, decoupling capacitors and connectors
- Portable and interactive: orientation sensor (BNO085 IMU), magnetic closing, 3D printed interior

The combination of a hand-held ball of LEDs with motion sensing opens a lot of interesting possibilities: motion light performances, visualizing data, interactive games, puzzles, etc.

- [v2 - level shifters](images/level-shifter.jpeg)
- [v2 - teensy 4.1 wiring](images/teensy-41.jpeg)
- [v2 - prototype internal wiring](images/prototype-internal.jpeg)

![wiring diagram](<images/teensy-41-wiring.png>)

In the final design, all of these components will be integrated into stackable PCBs designed to fit together like a puzzle. These would contain sensors, level shifters, power management, dual 18650 battery holders, and wireless charging. The Teensy will be mounted directly to this PCB.

![v2 - PCB design prototype](images/dodeca-interior-design.png)

## Getting Started

### Requirements

- [PlatformIO](https://platformio.org/) (IDE or CLI)
- A Teensy 4.1 with the DodecaRGB hardware wired up
- USB cable for flashing

### Flashing the Firmware

Clone the repo and open it in VSCode/Cursor with the PlatformIO extension, or use the CLI:

```bash
# Build and upload to Teensy 4.1
pio run -e teensy41 -t upload

# Monitor serial output
pio device monitor
```

### Configuration

The model geometry is defined in a YAML file at `src/models/DodecaRGBv2_1/model.yaml`. This file specifies face types, LED counts, rotation values, and wiring order. After editing, regenerate the C++ header:

```bash
python util/generate_model.py -d src/models/DodecaRGBv2_1 -y
```

**Calibrating face orientation:** Each face has a `rotation` value (0-4) representing 72-degree increments. If a face's LEDs appear rotated, adjust its rotation in `model.yaml` and regenerate. You can enable the `IdentifySidesScene` in `src/main.cpp` to display face IDs on each side, making it easy to verify which physical face maps to which ID.

See [Model System docs](docs/PixelTheater/Model.md) for details on face remapping and coordinate systems.

## Animations

The firmware ships with these scenes (in playlist order):

1. **Sparkles** - twinkling random lights
2. **Blob** - organic flowing shapes
3. **Boids** - flocking simulation
4. **Orientation Grid** - motion-reactive grid (uses IMU)
5. **Gravity Marbles** - physics-based rolling marbles (uses IMU)
6. **Satellites** - orbiting light trails
7. **Wandering Particles** - drifting particle system
8. **Texture Map** - spherical image projection (globe demo)
9. **XYZ Scanner** - axis-aligned scanning planes
10. **Geography** - world map projection

A button press advances to the next scene. The playlist is defined in the `setup()` function of `src/main.cpp` via `theater.addScene<>()` calls. To change the order, add, or remove scenes, edit those lines and rebuild.

To create your own animation, see the [Creating Animations Guide](docs/guides/creating_animations.md).

## The Software

The firmware is built in C++ using the Arduino framework and FastLED library. Animations are resolution-independent and implemented like shaders (given a pixel's 3D position, what color should it be?).

- **Firmware** (C++, Teensy 4.1): FastLED parallel output, [PixelTheater](docs/PixelTheater/README.md) animation framework, pre-calculated LED positions and neighbor distances, IMU support for motion-reactive features
- **Python Utilities** (`util/`): model generation from PCB pick-and-place files, 3D visualization, neighbor calculations, unit tests
- **Web Simulator**: browser-based testing environment using Emscripten/WebGL (see [Web Simulator Guide](docs/guides/web-simulator.md))

Each LED point knows its 3D coordinates, which face it belongs to, its nearest neighbors and their distances.

## Development

See the [Development Guide](docs/guides/development.md) for environment setup, testing, and tooling. The [Coding Guidelines](docs/guides/coding_guidelines.md) cover project standards.

### Testing

```bash
# C++ unit tests (native, no hardware needed)
pio test -e native

# Python utility tests
python -m util.tests.run_tests
```

### Python Utilities

The `util/` directory contains tools for model generation, 3D visualization, and coordinate calculations. See [util/README.md](util/README.md) for details.

## Project Structure

```
src/                  Firmware source code
  main.cpp            Entry point, scene registration, hardware setup
  scenes/             Animation scene implementations
  models/             Model YAML definitions and generated headers
lib/PixelTheater/     Animation framework library
docs/                 Documentation
  guides/             Development, animation creation, web simulator guides
  PixelTheater/       API reference for the PixelTheater library
util/                 Python utilities (model generation, viewer, tests)
web/                  Web simulator frontend
images/               Project images and diagrams
3d-models/            STL files for 3D printing
test/                 C++ test suites
examples/             Code examples (runtime API demo)
```

## Contributing

Pull requests are welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

This project is released under the [MIT License](LICENSE).

## Version 1

For info and assembly instructions for the original version (2023, 312 LEDs), see the [archived V1 readme](docs/guides/Dodeca-V1-info.md).
