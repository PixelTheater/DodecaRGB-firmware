#include "PixelTheater/color/palettes.h"
#include "PixelTheater/core/crgb.h" // For CRGB constants

// Use fully qualified names to define the constants declared in the header
namespace PixelTheater {
namespace Palettes {

// Define standard FastLED palettes using PixelTheater::CRGB
// Values copied from FastLED source colorpalettes.cpp

const CRGBPalette16 CloudColors = {
    CRGB::Blue,       CRGB::DarkBlue,   CRGB::DarkBlue,   CRGB::DarkBlue,
    CRGB::DarkBlue,   CRGB::DarkBlue,   CRGB::DarkBlue,   CRGB::DarkBlue,
    CRGB::Blue,       CRGB::DarkBlue,   CRGB::SkyBlue,    CRGB::SkyBlue,
    CRGB::LightBlue,  CRGB::White,      CRGB::LightBlue,  CRGB::SkyBlue
};

const CRGBPalette16 LavaColors = {
    CRGB::Black,    CRGB::Maroon,     CRGB::Black,      CRGB::Maroon,
    CRGB::DarkRed,  CRGB::DarkRed,    CRGB::Maroon,     CRGB::DarkRed,
    CRGB::DarkRed,  CRGB::DarkRed,    CRGB::Red,        CRGB::Orange,
    CRGB::White,    CRGB::Orange,     CRGB::Red,        CRGB::DarkRed
};

const CRGBPalette16 OceanColors = {
    CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::MidnightBlue, CRGB::Navy,
    CRGB::DarkBlue,     CRGB::MediumBlue, CRGB::SeaGreen,     CRGB::Teal,
    CRGB::CadetBlue,    CRGB::Blue,       CRGB::DarkCyan,     CRGB::CornflowerBlue,
    CRGB::Aquamarine,   CRGB::SeaGreen,   CRGB::Aqua,       CRGB::LightSkyBlue
};

const CRGBPalette16 ForestColors = {
    CRGB::DarkGreen,    CRGB::DarkGreen,    CRGB::DarkOliveGreen, CRGB::DarkGreen,
    CRGB::Green,        CRGB::ForestGreen,  CRGB::OliveDrab,      CRGB::Green,
    CRGB::SeaGreen,     CRGB::MediumAquamarine, CRGB::LimeGreen,    CRGB::YellowGreen,
    CRGB::LightGreen,   CRGB::LawnGreen,      CRGB::MediumAquamarine, CRGB::ForestGreen
};

// Use hex values for palettes defined that way in FastLED source
const CRGBPalette16 RainbowColors = {
    CRGB(0xFF0000), CRGB(0xD52A00), CRGB(0xAB5500), CRGB(0xAB7F00),
    CRGB(0xABAB00), CRGB(0x56D500), CRGB(0x00FF00), CRGB(0x00D52A),
    CRGB(0x00AB55), CRGB(0x0056AA), CRGB(0x0000FF), CRGB(0x2A00D5),
    CRGB(0x5500AB), CRGB(0x7F0081), CRGB(0xAB0055), CRGB(0xD5002B)
};

const CRGBPalette16 RainbowStripeColors = {
    CRGB(0xFF0000), CRGB(0x000000), CRGB(0xAB5500), CRGB(0x000000),
    CRGB(0xABAB00), CRGB(0x000000), CRGB(0x00FF00), CRGB(0x000000),
    CRGB(0x00AB55), CRGB(0x000000), CRGB(0x0000FF), CRGB(0x000000),
    CRGB(0x5500AB), CRGB(0x000000), CRGB(0xAB0055), CRGB(0x000000)
};

const CRGBPalette16 PartyColors = {
    CRGB(0x5500AB), CRGB(0x84007C), CRGB(0xB5004B), CRGB(0xE5001B),
    CRGB(0xE81700), CRGB(0xB84700), CRGB(0xAB7700), CRGB(0xABAB00),
    CRGB(0xAB5500), CRGB(0xDD2200), CRGB(0xF2000E), CRGB(0xC2003E),
    CRGB(0x8F0071), CRGB(0x5F00A1), CRGB(0x2F00D0), CRGB(0x0007F9)
};

const CRGBPalette16 HeatColors = {
    CRGB(0x000000),
    CRGB(0x330000), CRGB(0x660000), CRGB(0x990000), CRGB(0xCC0000), CRGB(0xFF0000),
    CRGB(0xFF3300), CRGB(0xFF6600), CRGB(0xFF9900), CRGB(0xFFCC00), CRGB(0xFFFF00),
    CRGB(0xFFFF33), CRGB(0xFFFF66), CRGB(0xFFFF99), CRGB(0xFFFFCC), CRGB(0xFFFFFF)
};

// --- Define Custom PixelTheater Palettes ---

const CRGBPalette16 basePalette = {
    CRGB::Red, CRGB::DarkRed, CRGB::IndianRed, CRGB::OrangeRed,
    CRGB::Green, CRGB::DarkGreen, CRGB::LawnGreen, CRGB::ForestGreen,
    CRGB::Blue, CRGB::DarkBlue, CRGB::SkyBlue, CRGB::Indigo,
    CRGB::Purple, CRGB::Indigo, CRGB::CadetBlue, CRGB::AliceBlue
}; 

const CRGBPalette16 highlightPalette = {
    CRGB::Yellow, CRGB::LightSlateGray, CRGB::LightYellow, CRGB::LightCoral, 
    CRGB::GhostWhite, CRGB::LightPink, CRGB::AntiqueWhite, CRGB::LightSkyBlue, 
    CRGB::Gold, CRGB::PeachPuff, CRGB::FloralWhite, CRGB::PaleTurquoise, 
    CRGB::Orange, CRGB::MintCream, CRGB::FairyLightNCC, CRGB::LavenderBlush
};

const CRGBPalette16 uniquePalette = {
    CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::Yellow, 
    CRGB::Purple, CRGB::Orange, CRGB::Cyan, CRGB::Magenta, 
    CRGB::Lime, CRGB::Pink, CRGB::Turquoise, CRGB::Sienna,
    CRGB::Gold, CRGB::Salmon, CRGB::Silver, CRGB::Violet
};

} // namespace Palettes
} // namespace PixelTheater 