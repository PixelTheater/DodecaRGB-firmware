#pragma once

#include "PixelTheater/SceneKit.h" // SceneKit helpers
#include <cmath>       // For std::sin, std::cos, std::abs
#include <algorithm>   // For std::clamp

namespace Scenes {

class SparklesScene : public Scene {
private:
    // Parameters handled by base class 'settings'

    // Palettes
    CRGBPalette16 palette1; // CloudColors
    CRGBPalette16 palette2; // HeatColors

    // Palette Index State
    uint8_t index1 = 128;
    uint8_t index2 = 128;
    float pos1 = 128.0f; // Current position for oscillation (around 128)
    float pos2 = 128.0f;
    float speed1 = 0.0f; // Current speed for oscillation
    float speed2 = 0.0f;

    // Color Blend State
    uint8_t color_blend = 128;
    float blend_pos = 128.0f; // Current position for oscillation (around 128)
    float blend_speed = 0.0f; // Current speed for oscillation

    // Chaos State
    float chaos_offset = 0.0f; // Current random offset (-1 to 1, scaled by chaos param)
    float chaos_target = 0.0f; // Target for smooth random walk
    uint32_t chaos_timer = 0;  // Timer for picking new chaos target

public:
    // Constructor
    SparklesScene() = default;

    // Required lifecycle methods
    void setup() override;
    void tick() override;

    // Optional status method
    std::string status() const override;
};

} // namespace Scenes 