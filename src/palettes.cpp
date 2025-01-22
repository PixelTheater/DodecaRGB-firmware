#include "palettes.h"

// application color palettes, hand picked for good contrast and exceptional taste of the author

CRGBPalette16 basePalette = CRGBPalette16( 
  CRGB::Red, CRGB::DarkRed, CRGB::IndianRed, CRGB::OrangeRed,
  CRGB::Green, CRGB::DarkGreen, CRGB::LawnGreen, CRGB::ForestGreen,
  CRGB::Blue, CRGB::DarkBlue, CRGB::SkyBlue, CRGB::Indigo,
  CRGB::Purple, CRGB::Indigo, CRGB::CadetBlue, CRGB::AliceBlue
); 
CRGBPalette16 highlightPalette = CRGBPalette16(
  CRGB::Yellow, CRGB::LightSlateGray, CRGB::LightYellow, CRGB::LightCoral, 
  CRGB::GhostWhite, CRGB::LightPink, CRGB::AntiqueWhite, CRGB::LightSkyBlue, 
  CRGB::Gold, CRGB::PeachPuff, CRGB::FloralWhite, CRGB::PaleTurquoise, 
  CRGB::Orange, CRGB::MintCream, CRGB::FairyLightNCC, CRGB::LavenderBlush
);
CRGBPalette16 uniquePalette = CRGBPalette16(
  CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::Yellow, 
  CRGB::Purple, CRGB::Orange, CRGB::Cyan, CRGB::Magenta, 
  CRGB::Lime, CRGB::Pink, CRGB::Turquoise, CRGB::Sienna,
  CRGB::Gold, CRGB::Salmon, CRGB::Silver, CRGB::Violet
);


// Array of named colors for lookup
const ColorName colorLookup[] = {
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


const size_t numLookupColors = sizeof(colorLookup) / sizeof(colorLookup[0]);

String getClosestColorName(const CRGB& color) {
    const ColorName* closest = &colorLookup[0];
    uint32_t closestDistance = colorDistance(color, colorLookup[0].color);

    for (size_t i = 1; i < numLookupColors; ++i) {
        uint32_t dist = colorDistance(color, colorLookup[i].color);
        if (dist < closestDistance) {
            closest = &colorLookup[i];
            closestDistance = dist;
        }
    }
    return String(closest->name);
}

uint32_t colorDistance(const CRGB& c1, const CRGB& c2) {
    int dr = c1.r - c2.r;
    int dg = c1.g - c2.g;
    int db = c1.b - c2.b;
    return dr * dr + dg * dg + db * db;
}

// Helper functions

// Function to convert RGB to ANSI 24-bit color codes
String getAnsiColorString(const CRGB& color, const char c) {  // default char is space
    char buf[64];
    CHSV inverted = rgb2hsv_approximate(color);
    inverted.v = 255 - inverted.v;
    CRGB invertedRGB = hsv2rgb_spectrum(inverted);
    snprintf(buf, sizeof(buf), 
        //"\033[38;2;%d;%d;%dm\033[48;2;%d;%d;%dm%c\033[0m",  // ANSI color escape sequences
        "\033[48;2;%d;%d;%dm%c\033[0m",  // ANSI color escape sequences
        //invertedRGB.r, invertedRGB.g, invertedRGB.b,  // foreground
        color.r, color.g, color.b,  // background
        c); // character
    return String(buf);
}

float get_perceived_brightness(const CHSV& color) {
    // Convert to RGB temporarily for luminance calculation
    CRGB rgb = CHSV(color.h, color.s, color.v);
    return (0.2126 * rgb.r + 0.7152 * rgb.g + 0.0722 * rgb.b) / 255.0;
}

float get_contrast_ratio(const CHSV& color1, const CHSV& color2) {
    float l1 = get_perceived_brightness(color1);
    float l2 = get_perceived_brightness(color2);
    float lighter = max(l1, l2);
    float darker = min(l1, l2);
    return (lighter + 0.05) / (darker + 0.05);
}

float get_hue_distance(const CHSV& color1, const CHSV& color2) {
    float diff = fabs(color1.h - color2.h);
    return min(diff, 255.0f - diff) * (180.0f/255.0f);  // convert to degrees (0-180)
}



