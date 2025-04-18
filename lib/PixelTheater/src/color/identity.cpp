#include "PixelTheater/color/identity.h"
#include "PixelTheater/color/measurement.h" // For colorDistance
#include "PixelTheater/core/crgb.h" // For CRGB type

#include <vector>
#include <string>
#include <cstdio> // For snprintf
#include <cstdint>

namespace PixelTheater {
namespace ColorUtils {

// Define the lookup table using the PixelTheater::CRGB type
// This table is used regardless of the platform
const std::vector<ColorName> colorLookup = {
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

std::string getClosestColorName(const PixelTheater::CRGB& color) {
    if (colorLookup.empty()) return "";

    const ColorName* closest = &colorLookup[0];
    uint32_t closestDistance = colorDistance(color, colorLookup[0].color);

    for (size_t i = 1; i < colorLookup.size(); ++i) {
        uint32_t dist = colorDistance(color, colorLookup[i].color);
        if (dist < closestDistance) {
            closest = &colorLookup[i];
            closestDistance = dist;
        }
        if (dist == 0) break; // Exact match found
    }
    return std::string(closest->name);
}

std::string getAnsiColorString(const PixelTheater::CRGB& color, const char c) {
    char buf[64];
    snprintf(buf, sizeof(buf),
             "\033[48;2;%d;%d;%dm%c\033[0m",
             color.r, color.g, color.b, c);
    return std::string(buf);
}

} // namespace ColorUtils
} // namespace PixelTheater 