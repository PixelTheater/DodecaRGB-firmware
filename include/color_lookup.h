#ifndef COLOR_LOOKUP_H
#define COLOR_LOOKUP_H

#include <FastLED.h>
#include <cmath>
#include <string>

// Struct for storing a color name and its RGB values
struct ColorName {
    const char* name;
    CRGB color;
};

// Array of colors from XKCD color survey
// Array of 64 evenly distributed colors
ColorName colorLookup[] = {
    {"Black", CRGB::Black},
    {"Red", CRGB::Red},
    {"Green", CRGB::Green},
    {"Blue", CRGB::Blue},
    {"Yellow", CRGB::Yellow},
    {"Cyan", CRGB::Cyan},
    {"Magenta", CRGB::Magenta},
    {"White", CRGB::White},
    {"Orange", CRGB::Orange},
    {"Purple", CRGB::Purple},
    {"Pink", CRGB::Pink},
    {"Aqua", CRGB::Aqua},
    {"Chartreuse", CRGB::Chartreuse},
    {"Coral", CRGB::Coral},
    {"Gold", CRGB::Gold},
    {"Lavender", CRGB::Lavender},
    {"Lime", CRGB::Lime},
    {"Maroon", CRGB::Maroon},
    {"Navy", CRGB::Navy},
    {"Olive", CRGB::Olive},
    {"Plum", CRGB::Plum},
    {"Salmon", CRGB::Salmon},
    {"SeaGreen", CRGB::SeaGreen},
    {"Sienna", CRGB::Sienna},
    {"Silver", CRGB::Silver},
    {"Teal", CRGB::Teal},
    {"Turquoise", CRGB::Turquoise},
    {"Violet", CRGB::Violet},
    {"Wheat", CRGB::Wheat},
    {"Crimson", CRGB::Crimson},
    {"DarkBlue", CRGB::DarkBlue},
    {"DarkGreen", CRGB::DarkGreen}
};

// Number of colors in the array
constexpr size_t numLookupColors = sizeof(colorLookup) / sizeof(colorLookup[0]);

// Function to calculate Euclidean distance
inline uint32_t colorDistance(const CRGB& c1, const CRGB& c2) {
    int dr = c1.r - c2.r;
    int dg = c1.g - c2.g;
    int db = c1.b - c2.b;
    return dr * dr + dg * dg + db * db;
}

// Function to find the closest color name
inline const char* getClosestColorName(const CRGB& color) {
    const ColorName* closest = &colorLookup[0];
    uint32_t closestDistance = colorDistance(color, colorLookup[0].color);

    for (size_t i = 1; i < numLookupColors; ++i) {
        uint32_t dist = colorDistance(color, colorLookup[i].color);
        if (dist < closestDistance) {
            closest = &colorLookup[i];
            closestDistance = dist;
        }
    }
    return closest->name;
}

#endif // COLOR_LOOKUP_H