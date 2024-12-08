#pragma once
#include <array>
#include <Arduino.h>
#include <FastLED.h>

// Color palettes
// as an array of named palettes, each containing 6 CRGB colors. 

class ColorPalette {
  public:
    String name;
    CRGB colors[6];

    ColorPalette(const char *name, std::array<CRGB, 6> colors);
};

// Color palettes
// as an array of named palettes, each containing 6 CRGB colors.
//
// The palettes are defined in palettes.cpp
//

extern ColorPalette *palettes[];
extern const int paletteCount;



