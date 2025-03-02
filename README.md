# DodecaRGB V2

Jan 2025: *V2 in development!* Version two introduces higher density micro-pixels (1248 in total!) and a slightly smaller size overall. Click below to see a teaser video.

[![DodecaRGB v2 Teaser Video](images/yt-preview-thumb.png)](https://www.youtube.com/watch?v=RErgt5O7D7U)

## Vision

To create an awesome open-source animation and interaction platform for running resolution-indpendent apps on portable, interactive 3D physical models. And because we love making things that have lots of LEDs.

## Realization

DodecaRGB V2 is a modular, high-resolution programmable blinky gadget that was made for the hacker community, especially for coders that want to do spherical animations. Typically, this is quite hard - you either need to make a spinning POV globe, or wire up custom solutions - not to mention power and performance challenges.

This project aims to create a standard platform for resolution-indpendent animations based on spherical models (a developing library here called [PixelTheater](docs/PixelTheater.md)). DodecaRGB V2 is the second iteration of this platform, and serves as a reference implementation.

The firmware, 3d models and tooling are open source and free to use and modify. The hardware PCBs will are planned as a kit for 2025, which will include most of the parts needed to 3d print and assemble your own.

## The Reference LED Model

- We have a dodecahedron model with 12 sides.
- Each side is a pentgon-shaped PCB circuit board that contains 104 RGB leds.
- Each side connects to the next, in series, for a grand total of 1248 LEDs.
- The whole thing runs at >50fps, and can be battery powered
- Portable and interactive: wireless charging, orientation sensor, magnetic closing, 3d printed interior (soon)
- Other ideas may extend the model (haptic feedback, sound reactive, speaker and sound engine, etc)

The combination of a hand-held ball of LEDs with motion sensing opens a lot of interesting user interaction experiments: motion light performances, visualizing data, interactive games, puzzles, etc.

> This reference platform is planned to be available as a kit for mid-2025, but the implementation is open
> source and could be applied to other models with different shapes and LED layouts.

![fits in the hand](<images/v2-juggle.gif>)

## The Hardware

**Note: These are prototypes, and the final hardware will be a bit more *refined and tested*.**

- 2-layer PCBs with the WS2812 LEDs (SMD 1615 package), decoupling capacitors and connectors
- A Teensy 4.1 microcontroller is used to control everything
- Level shifters (for LEDs), power regulation, battery, etc.
- FastLED parallel support is being used (see <https://github.com/FastLED/FastLED/releases/tag/3.9.9>)
- The two hemispheres of the model are wired on separate channels, using pins 19 and 18 of the Teensy, so 624 LEDs per channel. This allows for higher frame rates.

- [v2 - level shifters](images/level-shifter.jpeg)
- [v2 - teensy 4.1 wiring](images/teensy-41.jpeg)
- [v2 - prototype internal wiring](images/prototype-internal.jpeg)

![wiring diagram](<images/teensy-41-wiring.png>)

In the final design, all of these components will be integrated into stackable PCBs designed to fit together like a puzzle. These would contain sensors, level shifters, power management, dual 18650 battery holders, and wireless charging. The Teensy will be mounted directly to this PCB.

![v2 - PCB design prototype](images/dodeca-interior-design.png)

## The Software

The firmware is built in C++ using the Arduino framework and FastLED library. In general, animations are resolution-independent and are implemented like shaders (given a pixel, what color should it be?).

- **Firmware**: C++ code running on the Teensy 4.1
  - FastLED for LED control with parallel output support
  - Object-oriented animation framework
  - Pre-calculated LED positions and neighbor distances
  - IMU support for motion-reactive features

- **Python Utilities** (`util/` directory):
  - 3D visualization tool for development and testing
  - LED coordinate generation from PCB pick-and-place files
  - Neighbor distance calculations and validation
  - Unit tests for all core functionality

The LED positions are generated from the PCB pick-and-place files, transformed into 3D space, and exported as C++ code. The visualizer helps verify the positions and side configurations before generating the final code. This makes cool animations easier to create.

Each LED point knows its:

- 3D coordinates (x, y, z)
- Which face it belongs to (0-11)
- Its nearest neighbors and their distances
- Original PCB position and label

There is a [developer overview](docs/development.md) as well as documentation on [creating animations](docs/creating_animations.md).

## Testing

The project includes unit tests for the cpp classes and python utilities. See [Development.md](docs/Development.md) for more information.

## TODOs

### 🚧 In Progress

- 📱 Motion-reactive support
  - Evaluating different IMU choices
  - Investigating orientation and sensitivity options
  - Planning gesture support (tap, shake, spin, etc.)
- 🎨 3D modelling of interior structure
- 🔌 Motherboard PCB development
  - Teensy 4.1 mounting
  - 2x 18650 battery integration
  - Power and charging circuits
  - Level shifting
  - Sensor integration

### 🎯 Future Plans

- 🔋 Wireless charging (Qi), testing different coils
  - Optimizing PCB placement for production
- 🖼️ Image loading support (per-side images, mapping projections for globes and spherical photos)
  - Per-side images (for each face, for UI presentation)
- ⚡ Hardware design improvements for wiring, closure, durability, etc.
  - Step-by-step assembly instructions with photos, or video tutorial

### ✅ Recently Completed

- ✓ Refactoring animations into modules
- ✓ Updated and expanded documentation
- ✓ Isolated and standardized interfaces
  ✓  Simplified playlist and animation creation
- ✓ Migrated coordinate and code generation to Python utilities
- ✓ Added unit tests for Python utilities

### Version 1 (2023)

For info and assembly instructions for version 1, see [the archived readme](docs/Dodeca-V1-info.md).

## Development Tools

The project uses the PixelTheater animation system (part of this project) to manage animations. Full docs are in [docs/PixelTheater.md](docs/PixelTheater.md).

Info about doing development with this project is available in [Development.md](docs/Development.md). (pull requests welcome!)

The python utilities are documented in [util/README.md](util/README.md). This includes how to generate the LED coordinates and visualizer from the PCB pick-and-place files, along with unit tests for the utilities.


