#pragma once
#include <cstdint>
#include <algorithm>  // For std::fill
#include "color_utils.h"  // For helper functions

namespace PixelTheater {

// Forward declarations
class CRGB;

class CHSV {
public:
    union {
        struct {
            union {
                uint8_t h;
                uint8_t hue;
            };
            union {
                uint8_t s;
                uint8_t sat;
                uint8_t saturation;
            };
            union {
                uint8_t v;
                uint8_t val;
                uint8_t value;
            };
        };
        uint8_t raw[3];
    };

    // Constructors
    CHSV() : h(0), s(0), v(0) {}
    CHSV(uint8_t hue, uint8_t sat, uint8_t val) : h(hue), s(sat), v(val) {}

    // Common HSV color points
    static const uint8_t HUE_RED = 0;
    static const uint8_t HUE_ORANGE = 32;
    static const uint8_t HUE_YELLOW = 64;
    static const uint8_t HUE_GREEN = 96;
    static const uint8_t HUE_AQUA = 128;
    static const uint8_t HUE_BLUE = 160;
    static const uint8_t HUE_PURPLE = 192;
    static const uint8_t HUE_PINK = 224;
};

class CRGB {
public:
    union {
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };
        struct {
            uint8_t red;
            uint8_t green;
            uint8_t blue;
        };
        uint8_t raw[3];
    };

    // Simple initialization
    constexpr CRGB() : r(0), g(0), b(0) {}
    constexpr CRGB(uint8_t r_, uint8_t g_, uint8_t b_) : r(r_), g(g_), b(b_) {}
    CRGB(uint32_t colorcode) {
        r = (colorcode >> 16) & 0xFF;
        g = (colorcode >> 8) & 0xFF;
        b = colorcode & 0xFF;
    }

    // Allow construction from HSV
    inline CRGB(const CHSV& rhs) {
        hsv2rgb_rainbow(rhs, *this);
    }

    // Explicitly define assignment operator
    inline CRGB& operator= (const CRGB& rhs) {
        r = rhs.r;
        g = rhs.g;
        b = rhs.b;
        return *this;
    }

    // Also need to explicitly define other assignment operators
    inline CRGB& operator= (const uint32_t colorcode) {
        r = (colorcode >> 16) & 0xFF;
        g = (colorcode >> 8) & 0xFF;
        b = colorcode & 0xFF;
        return *this;
    }

    inline CRGB& operator= (const CHSV& rhs) {
        hsv2rgb_rainbow(rhs, *this);
        return *this;
    }

    // Utility method for setting RGB values
    inline CRGB& setRGB(uint8_t nr, uint8_t ng, uint8_t nb) {
        r = nr;
        g = ng;
        b = nb;
        return *this;
    }

    // Array access (both const and non-const versions)
    inline uint8_t& operator[] (uint8_t x) { return raw[x]; }
    inline const uint8_t& operator[] (uint8_t x) const { return raw[x]; }

    // Add math operators
    inline CRGB& operator+= (const CRGB& rhs) {
        r = qadd8(r, rhs.r);
        g = qadd8(g, rhs.g); 
        b = qadd8(b, rhs.b);
        return *this;
    }

    inline CRGB& operator-= (const CRGB& rhs) {
        r = qsub8(r, rhs.r);
        g = qsub8(g, rhs.g);
        b = qsub8(b, rhs.b);
        return *this;
    }

    inline CRGB& operator*= (uint8_t scale) {
        if(scale == 0) {
            r = g = b = 0;
        } else if(scale != 255) {
            r = scale8(r, scale);
            g = scale8(g, scale);
            b = scale8(b, scale);
        }
        return *this;
    }

    // Comparison operators
    bool operator==(const CRGB& rhs) const {
        return r == rhs.r && g == rhs.g && b == rhs.b;
    }

    bool operator!=(const CRGB& rhs) const {
        return !(*this == rhs);
    }

    // basic colors
    static const CRGB Black;
    static const CRGB White;
    static const CRGB Red;
    static const CRGB Green;
    static const CRGB Blue;
    static const CRGB Yellow;
    static const CRGB Magenta;
    static const CRGB Cyan;

    // extended colors
    static const CRGB AliceBlue;
    static const CRGB Amethyst;
    static const CRGB AntiqueWhite;
    static const CRGB Aqua;
    static const CRGB Aquamarine;
    static const CRGB Azure;
    static const CRGB Beige;
    static const CRGB Bisque;
    static const CRGB BlanchedAlmond;
    static const CRGB BlueViolet;
    static const CRGB Brown;
    static const CRGB BurlyWood;
    static const CRGB CadetBlue;
    static const CRGB Chartreuse;
    static const CRGB Chocolate;
    static const CRGB Coral;
    static const CRGB CornflowerBlue;
    static const CRGB Cornsilk;
    static const CRGB Crimson;
    static const CRGB DarkBlue;
    static const CRGB DarkCyan;
    static const CRGB DarkGoldenrod;
    static const CRGB DarkGray;
    static const CRGB DarkGrey;
    static const CRGB DarkGreen;
    static const CRGB DarkKhaki;
    static const CRGB DarkMagenta;
    static const CRGB DarkOliveGreen;
    static const CRGB DarkOrange;
    static const CRGB DarkOrchid;
    static const CRGB DarkRed;
    static const CRGB DarkSalmon;
    static const CRGB DarkSeaGreen;
    static const CRGB DarkSlateBlue;
    static const CRGB DarkSlateGray;
    static const CRGB DarkSlateGrey;
    static const CRGB DarkTurquoise;
    static const CRGB DarkViolet;
    static const CRGB DeepPink;
    static const CRGB DeepSkyBlue;
    static const CRGB DimGray;
    static const CRGB DimGrey;
    static const CRGB DodgerBlue;
    static const CRGB FireBrick;
    static const CRGB FloralWhite;
    static const CRGB ForestGreen;
    static const CRGB Fuchsia;
    static const CRGB Gainsboro;
    static const CRGB GhostWhite;
    static const CRGB Gold;
    static const CRGB Goldenrod;
    static const CRGB Gray;
    static const CRGB Grey;
    static const CRGB GreenYellow;
    static const CRGB Honeydew;
    static const CRGB HotPink;
    static const CRGB IndianRed;
    static const CRGB Indigo;
    static const CRGB Ivory;
    static const CRGB Khaki;
    static const CRGB Lavender;
    static const CRGB LavenderBlush;
    static const CRGB LawnGreen;
    static const CRGB LemonChiffon;
    static const CRGB LightBlue;
    static const CRGB LightCoral;
    static const CRGB LightCyan;
    static const CRGB LightGoldenrodYellow;
    static const CRGB LightGreen;
    static const CRGB LightGrey;
    static const CRGB LightPink;
    static const CRGB LightSalmon;
    static const CRGB LightSeaGreen;
    static const CRGB LightSkyBlue;
    static const CRGB LightSlateGray;
    static const CRGB LightSlateGrey;
    static const CRGB LightSteelBlue;
    static const CRGB LightYellow;
    static const CRGB Lime;
    static const CRGB LimeGreen;
    static const CRGB Linen;
    static const CRGB Maroon;
    static const CRGB MediumAquamarine;
    static const CRGB MediumBlue;
    static const CRGB MediumOrchid;
    static const CRGB MediumPurple;
    static const CRGB MediumSeaGreen;
    static const CRGB MediumSlateBlue;
    static const CRGB MediumSpringGreen;
    static const CRGB MediumTurquoise;
    static const CRGB MediumVioletRed;
    static const CRGB MidnightBlue;
    static const CRGB MintCream;
    static const CRGB MistyRose;
    static const CRGB Moccasin;
    static const CRGB NavajoWhite;
    static const CRGB Navy;
    static const CRGB OldLace;
    static const CRGB Olive;
    static const CRGB OliveDrab;
    static const CRGB Orange;
    static const CRGB OrangeRed;
    static const CRGB Orchid;
    static const CRGB PaleGoldenrod;
    static const CRGB PaleGreen;
    static const CRGB PaleTurquoise;
    static const CRGB PaleVioletRed;
    static const CRGB PapayaWhip;
    static const CRGB PeachPuff;
    static const CRGB Peru;
    static const CRGB Pink;
    static const CRGB Plaid;
    static const CRGB Plum;
    static const CRGB PowderBlue;
    static const CRGB Purple;
    static const CRGB RosyBrown;
    static const CRGB RoyalBlue;
    static const CRGB SaddleBrown;
    static const CRGB Salmon;
    static const CRGB SandyBrown;
    static const CRGB SeaGreen;
    static const CRGB Seashell;
    static const CRGB Sienna;
    static const CRGB Silver;
    static const CRGB SkyBlue;
    static const CRGB SlateBlue;
    static const CRGB SlateGray;
    static const CRGB SlateGrey;
    static const CRGB Snow;
    static const CRGB SpringGreen;
    static const CRGB SteelBlue;
    static const CRGB Tan;
    static const CRGB Teal;
    static const CRGB Thistle;
    static const CRGB Tomato;
    static const CRGB Turquoise;
    static const CRGB Violet;
    static const CRGB Wheat;
    static const CRGB WhiteSmoke;
    static const CRGB YellowGreen;

    static const CRGB FairyLight;
    static const CRGB FairyLightNCC;
};

// Common colors namespace
namespace Colors {
    extern const CRGB White;
    extern const CRGB Red;
    extern const CRGB Green;
    extern const CRGB Blue;
    extern const CRGB Black;
}

// HSV-RGB conversion operators (these belong with CRGB since they're core conversion)
inline CRGB operator|(const CHSV& hsv, const CRGB&) {
    CRGB rgb;
    hsv2rgb_rainbow(hsv, rgb);
    return rgb;
}

inline CRGB& operator%=(CRGB& rgb, const CHSV& hsv) {
    hsv2rgb_rainbow(hsv, rgb);
    return rgb;
}

} // namespace PixelTheater 