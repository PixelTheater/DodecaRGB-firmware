# DodecaRGB Coding Guidelines

This serves as documentation for chosen best practices and standards that all developers should follow for this project.

## Overview

DodecaRGB is a project to enable the creation and orchestration of multiple colorful animations for a hardware-based LED object.
Animations are structured in a 360-degree environment, with support for different coordinate strategies: spherical, cartesian, 
and connected point graphs.

### The hardware

- A Teensy 4.1 microcontroller drives 1,248 individually addressable RGB LEDs arranged as follows:
  - 12 identical pentagon-shaped PCBs with 104 LEDs each
  - All PCBs assembled form a dodecahedron which is around 13cm in diameter
  - Each face's LEDs can be controlled independently using FastLED, addressible as one long strip 
- IMU (accelerometer, gyroscope, magnetometer)
- Pushbutton (for advancing control or resetting)

### The main application (the "firmware")

- Developed in C++ using the Arduino framework
- main.cc is where setup() initializes and loop() runs the animation environment
- palettes.cpp contains the color palettes and color functions
- Each animation is defined in a separate .cpp file in the animations/ directory and may 
- The Point class in points.cpp defines every LED pixel and their X,Y,Z coordinates, and which face they belong to as well as polar coordinate functions. It can also find nearest neighbors and measure distances.
- The Particle class in particles.cpp defines the particles that are animated with lifespan, velocity, and acceleration.

## Development Guidelines

### Code Style and Structure

- Write concise, idiomatic C++ code with accurate examples.
- Follow modern C++ conventions and best practices.
- Use object-oriented, procedural, or functional programming patterns as appropriate.
- Leverage STL and standard algorithms for collection operations.
- Use descriptive variable and method names (e.g., 'isUserSignedIn', 'calculateTotal').
- Structure files into headers (*.hpp) and implementation files (*.cpp) with logical separation of concerns.

### Naming Conventions

- Use PascalCase for class names.
- Use camelCase for variable names and methods.
- Use SCREAMING_SNAKE_CASE for constants and macros.
- Prefix member variables with an underscore or m_ (e.g., `_speed`).
- Use namespaces to organize code logically.

### C++ Features Usage

- Prefer modern C++ features (e.g., auto, range-based loops, smart pointers).
- Use `std::unique_ptr` and `std::shared_ptr` for memory management.
- Prefer `std::optional`, `std::variant`, and `std::any` for type-safe alternatives.
- Use `constexpr` and `const` to optimize compile-time computations.
- Use `std::string_view` for read-only string operations to avoid unnecessary copies.

### Syntax and Formatting

- Follow a consistent coding style, suggest improvements for clarity and consistency.
- Place braces on the same line for control structures and methods.
- Use clear and consistent commenting practices.

### Error Handling and Validation

- Use exceptions for error handling (e.g., `std::runtime_error`, `std::invalid_argument`).
- Use RAII for resource management to avoid memory leaks.
- Validate inputs at function boundaries.
- Log errors using Serial.printf()

### Performance Optimization

- Avoid unnecessary heap allocations; prefer stack-based objects where possible.
- Use `std::move` to enable move semantics and avoid copies.
- Optimize loops with algorithms from `<algorithm>` (e.g., `std::sort`, `std::for_each`).

### Key Conventions

- Use smart pointers over raw pointers for better memory safety.
- Use `enum class` for strongly typed enumerations.
- Separate interface from implementation in classes unless the class is very simple.
- Use templates and metaprogramming judiciously for generic solutions.

### Testing

- Aim to structure code in a way that is easy to test.
- Write unit tests using Catch2.
- Firmware can be hard to test as the arduino framework is not easy to mock. 
- Aim to test modules or classes in isolation, and use the physical model to test the integration.

### Security

- Use secure coding practices to avoid vulnerabilities (e.g., buffer overflows, dangling pointers).
- Prefer `std::array` or `std::vector` over raw arrays.
- Avoid C-style casts; use `static_cast`, `dynamic_cast`, or `reinterpret_cast` when necessary.
- Enforce const-correctness in functions and member variables.

Follow the official ISO C++ standards and guidelines for best practices in modern C++ development.

## Utilities and Build System

The util/ directory contains python helper scripts for visualizing the physical model, generating the LED coordinates and other pre-calculation tasks, as well as importing and exporting different files used (such as CSV pick-and-place files, json, etc). 

As each face of the dodecahedron is a pentagon that could be rotated in 5 different positions. This configuration is defined in the side_rotation[] array, where each side has a number 1-5 indication on of 5 possible rotations in 72 degree increments.

## Documentation

- Write clear comments for classes, methods, and critical logic.
- Document assumptions, constraints, and expected behavior of code.
- Keep documentation short and factual, only add comments that improve clarity
- Add comments in code to point out known problems, limitations, or common pitfalls.
- After any major changes, update the developer guide and tutorial creating_animations.md.
- After any major changes, update the README.md with any new or changed information.
- After any major changes, update the VERSION variable in the main.cc file.