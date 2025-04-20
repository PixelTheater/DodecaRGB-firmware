# PixelTheater Easier API Refactoring - Project Plan

## Palette Restructuring (Platform Transparent API)

### Goal Summary

To provide a simple, reliable, and **platform-transparent** way for scenes to use color palettes and utilities. Scenes will interact solely with the `PixelTheater` namespace, which will internally handle platform differences (FastLED on Teensy, C++ emulation/fallbacks elsewhere) without requiring conditional compilation within scene code.

### Core Principles

1.  **Consistent `PixelTheater` Namespace**: All types (`CRGB`, `CHSV`, `CRGBPalette16`) and core functions (`colorFromPalette`, `blend`, etc.) used by scenes reside within and are accessed via the `PixelTheater` namespace.
2.  **Internal Platform Handling**: The *implementation* of `PixelTheater` types and functions will conditionally use FastLED natives (when `PLATFORM_TEENSY`) or C++ fallback/emulation logic.
3.  **API Abstraction**: Scenes include only `PixelTheater` headers, insulating them from underlying FastLED details or fallback implementations.
4.  **Unified Palette Access**: All standard FastLED palettes and project-specific custom palettes will be exposed as `constexpr` or `extern const` constants within the `PixelTheater::Palettes` namespace (e.g., `PixelTheater::Palettes::Rainbow`, `PixelTheater::Palettes::basePalette`).

