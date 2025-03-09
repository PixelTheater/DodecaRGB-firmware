---
category: Development
generated: 2025-02-10 00:32
version: 2.8.2
---

# Palette System

## Overview

The palette system provides a way to define, store and use color palettes for LED animations. It uses the WLED palette format which is compatible with FastLED's gradient palettes.

## Lifecycle

1. Palettes are defined as JSON files in the `palettes` directory (by scene, or globally)
2. Scenes define parameters that can reference palettes
3. During build:
    - Python script validates JSON files (format, indices, etc)
    - Generates separate const structs for each palette
    - Generates simple name lookup function
4. When the firmware is built, the palettes are compiled into the image
5. At runtime:
    - Scene config() validates palette names exist
    - settings() returns palette data ready for FastLED use

## Palette Format

Palettes are stored in JSON files with a .pal.json extension:

```json
{
  "name": "Ocean Breeze",
  "description": "Cool blues and cyan tones",
  "palette": [
    0,   0,   0,   128,   // Dark blue at 0%
    64,  0,   255, 255,   // Cyan at 25% 
    128, 255, 255, 255,   // White at 50%
    192, 0,   255, 255,   // Cyan at 75%
    255, 0,   0,   128    // Dark blue at 100%
  ]
}
```

The palette file format `*.pal.json` is compatible with [WLED Custom Palettes](https://kno.wled.ge/features/palettes/#custom-palettes).

Each entry in the palette array is 4 numbers:

- Position (0-255, maps to 0-100%)
- Red (0-255)
- Green (0-255)
- Blue (0-255)

## Usage in Scenes

```cpp
void config() override {
    // Reference palette by name - only validates existence
    param("palette", Palette, "ocean");
}

void tick() override {
    // Get palette data and create FastLED palette
    CRGBPalette16 fastled_pal(pal_data);

    // Use FastLED palette directly
    CRGB color = ColorFromPalette(fastled_pal, position * 255);
}
```

## Built-in Palettes

The system includes the [FastLED Predefined Palettes](https://fastled.io/docs/group___predefined_palettes.html): `CloudColors_p, LavaColors_p, OceanColors_p, ForestColors_p, RainbowColors_p, RainbowStripeColors_p, PartyColors_p, HeatColors_p`.

## Creating Custom Palettes

Custom palettes can be:

1. Added to the global props.yaml:

```yaml
props:
  palettes:
    custom_pal:
      file: palettes/custom.pal.json
```

2. Added to a scene's props:

```yaml
props:
  scene_pal:
    file: props/scene.pal.json
```

Palettes can also be imported from WLED's [PaletteKnife tool](http://fastled.io/tools/paletteknife/), which use a simple json format.

TODO (idea): support CSS3 gradient palettes.

## Palettes and the Build System

The build system will:

1. Load .pal.json files
2. Convert to FastLED gradient format
3. Generate C++ palette data
4. Make palettes available to scenes

```cpp
// Generated palette_data.h (C++) from pallete file (json)
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
