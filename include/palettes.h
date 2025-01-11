#pragma once
#include <array>
#include <Arduino.h>
#include <FastLED.h>

// Struct for storing a color name and its RGB values
struct ColorName {
    const char* name;
    CRGB color;
};

// Color lookup array declaration
extern const ColorName colorLookup[];
extern const size_t numLookupColors;

// Color utility functions
String getAnsiColorString(const CRGB& color);
String getClosestColorName(const CRGB& color);
uint32_t colorDistance(const CRGB& c1, const CRGB& c2);

// Helper functions for color operations
float get_perceived_brightness(const CHSV& color);
float get_contrast_ratio(const CHSV& color1, const CHSV& color2);
float get_hue_distance(const CHSV& color1, const CHSV& color2);

// Global palettes
extern CRGBPalette16 basePalette;
extern CRGBPalette16 highlightPalette;
extern CRGBPalette16 uniquePalette;




