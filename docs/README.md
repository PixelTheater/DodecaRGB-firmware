---
category: Guide
generated: 2025-02-13 18:48
version: 2.8.3
---

# DodecaRGB and PixelTheater Documentation

This directory contains documentation for the DodecaRGB and PixelTheater projects.

The project is built in C++ and uses the Arduino framework, FastLED, and several other libraries. In general, common patterns around the stdlib are use, such as std::string, std::vector, std::map, std::unique_ptr, etc. In addition the Eigen library is used for linear algebra. PlatformIO or Cursor IDE are assumed for development.

The DodecaRGB firmware can load multiple animations and switch between them. Each animation is like a small shader program - it runs every frame and updates the LED colors based where it is on the display. This happens 50+ times per second, and the addressable LEDs are updated in parallel. The animation framework provides common functionality and patterns for defining animations, handling settings, presets, playlists, color palettes, status messages, and more.

## PixelTheater

- [PixelTheater](PixelTheater/README.md) - A detailed guide to the PixelTheater animation system
- [Palettes](PixelTheater/Palettes.md) - All about color workflow and importing palettes
- [Parameters](PixelTheater/Parameters.md) - A guide to the parameter system used in PixelTheater
- [Build System](PixelTheater/build-system.md) - An overview of the build system used to generate the C++ code from the YAML scene definitions (and more)
- [Python Utilities](../util/README.md) - A guide to the Python utilities used in the project (parameter generation, point calculations, visualization)

## HowTo Guides

- [Creating Animations](creating_animations.md) - general guide the covers how to craft different kinds of scenes using various coordinate systems and effects, plus some tips and techniques
- [Development](development.md) - A guide to setting up the development environment and building the project

## Other Things

- [Coding Guidelines](coding_guidelines.md) - A guide to the coding standards and practices used in the project
- [Changelog](../Changelog.md) - A list of changes and improvements for each version of the project
- [Dodeca V1 info](Dodeca-V1-info.md) - Information about the first iteration of the DodecaRGB hardware (2023)
