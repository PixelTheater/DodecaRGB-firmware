#pragma once
#include <cstdint>
#include "color_utils.h"  // For helper functions

namespace PixelTheater {

class CRGB {
public:
public:
    union {
        struct {
            union {
                uint8_t r;
                uint8_t red;
            };
            union {
                uint8_t g; 
                uint8_t green;
            };
            union {
                uint8_t b;
                uint8_t blue;
            };
        };
        uint8_t raw[3];
    };

    // Constructors
    CRGB() : r(0), g(0), b(0) {}
    CRGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
    CRGB(uint32_t colorcode) {
        r = (colorcode >> 16) & 0xFF;
        g = (colorcode >> 8) & 0xFF;
        b = colorcode & 0xFF;
    }

    // Allow construction from HSV
    inline CRGB(const CHSV& rhs) {
        hsv2rgb_rainbow(rhs, *this);
    }

    // Allow copy construction
    inline CRGB(const CRGB& rhs) = default;

    // Assignment operators
    inline CRGB& operator= (const CRGB& rhs) = default;
    
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

    // Common operations
    void fadeToBlackBy(uint8_t amount) {
        // Add 1 to handle rounding
        r = ((uint16_t)r * (256 - amount)) >> 8;
        g = ((uint16_t)g * (256 - amount)) >> 8;
        b = ((uint16_t)b * (256 - amount)) >> 8;
    }

    void fadeLightBy(uint8_t amount) {
        fadeToBlackBy(amount);
    }

    uint8_t getAverageLight() const {
        return (r + g + b) / 3;
    }

    void nscale8(uint8_t scale) {
        if (scale == 255) {
            // No change needed
            return;
        }
        if (scale == 0) {
            // Fast path to black
            r = g = b = 0;
            return;
        }
        // Use uint16_t to prevent overflow and match FastLED behavior
        r = ((uint16_t)r * (1 + scale)) >> 8;
        g = ((uint16_t)g * (1 + scale)) >> 8;
        b = ((uint16_t)b * (1 + scale)) >> 8;
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

} // namespace PixelTheater 