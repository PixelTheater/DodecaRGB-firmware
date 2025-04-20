# PixelTheater Easier API Refactoring - Project Plan

## Palette Restructuring (Platform Transparent API)

### Goal Summary

To provide a simple, reliable, and **platform-transparent** way for scenes to use color palettes and utilities. Scenes will interact solely with the `PixelTheater` namespace, which will internally handle platform differences (FastLED on Teensy, C++ emulation/fallbacks elsewhere) without requiring conditional compilation within scene code.

### Core Principles

1.  **Consistent `PixelTheater` Namespace**: All types (`CRGB`, `CHSV`, `CRGBPalette16`) and core functions (`colorFromPalette`, `blend`, etc.) used by scenes reside within and are accessed via the `PixelTheater` namespace.
2.  **Internal Platform Handling**: The *implementation* of `PixelTheater` types and functions will conditionally use FastLED natives (when `PLATFORM_TEENSY`) or C++ fallback/emulation logic.
3.  **API Abstraction**: Scenes include only `PixelTheater` headers, insulating them from underlying FastLED details or fallback implementations.
4.  **Unified Palette Access**: All standard FastLED palettes and project-specific custom palettes will be exposed as `constexpr` or `extern const` constants within the `PixelTheater::Palettes` namespace (e.g., `PixelTheater::Palettes::Rainbow`, `PixelTheater::Palettes::basePalette`).

### SceneKit Convenience Header

To simplify scene authoring we will add `PixelTheater/SceneKit.h`.

Goals
- Eliminate repetitive `PixelTheater::` qualifiers in scene code.
- Provide a single, stable set of aliases for the most frequently used engine
  symbols (Scene, CRGB, CHSV, palette types, PT_PI constants, map()).
- Keep aliases inside `namespace Scenes` to avoid leaking into the global
  namespace or colliding with FastLED symbols.
- Exclude any asset‑specific types (e.g. `TextureData`), which are included by
  the individual scenes that need them.

Usage Pattern
```cpp
#include "PixelTheater/SceneKit.h"
#include "textures/texture_data.h"    // if a particular scene needs it

namespace Scenes {
class MyScene : public Scene { … };
}
```

All built‑in demo scenes will be migrated to use SceneKit; new user scenes
should do the same.  This removes nearly all fully‑qualified names while
preserving a clean architectural boundary.

Current helper aliases provided by SceneKit (v0.2):

- Core types: `Scene`, `CRGB`, `CHSV`, `CRGBPalette16`
- Math helpers: `map`, `PT_PI`, `PT_TWO_PI`, `PT_HALF_PI`
- Colour helpers: `scale8_video`, `blend8`, `fadeToBlackBy`, `nblend`

Migrated demo scenes so far:

| Scene | Teensy | Web |
|-------|--------|-----|
| TextureMapScene | ✅ | ✅ |
| WanderingParticlesScene | ✅ | ✅ |
| BlobScene | ✅ | ✅ |

Next targets: `BoidsScene`, `OrientationGridScene`, `Sparkles`, `XYZScanner`, `Geography`.

### Essential Information

Native tests are run with: `~/.platformio/penv/bin/pio test -e native`