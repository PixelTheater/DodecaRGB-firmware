#pragma once

#include "PixelTheater/SceneKit.h"
#include <cmath> // For std::atan2, std::acos, std::fmod, std::min, std::abs, std::round, std::sin, std::cos

// Use shorter Eigen types (Available via ArduinoEigen dependency)
using Vector3f = Eigen::Vector3f;
using Matrix3f = Eigen::Matrix3f;

// No global using directives to avoid name clashes

namespace Scenes {

class OrientationGridScene : public Scene {
private:
    // --- Parameters (Cached from settings) ---
    int lat_lines_ = 5;
    int lon_lines_ = 4;
    uint16_t cycle_time_frames_ = 1500; // Use frames instead of ms for simplicity with tickCount()
    uint16_t transition_duration_frames_ = 200;
    float target_line_width_ = 0.14f;
    float previous_target_line_width_ = 0.14f; // Store width before transition

    // --- Colors ---
    CRGB bg_color_;
    CRGB line_color_;
    CRGB target_bg_color_;
    CRGB target_line_color_;
    CRGB bg_color_prev_;
    CRGB line_color_prev_;

    // --- State ---
    bool dark_lines_ = true;
    bool in_transition_ = true;
    size_t transition_timer_ = 0; // Frame counter for transitions
    float current_line_width_ = 0.14f;

    // --- Animation ---
    float rotation_angle_ = 0.0f;
    float tilt_speed_ = 0.0f; // Accumulated value, maybe rename?

    // --- Helper Method Declarations ---
    void pickNewColors();
    void blendToTarget(float blend_amount_0_1);
    static float angleDiff(float a1, float a2); // Static helper declaration

public:
    OrientationGridScene() = default;
    ~OrientationGridScene() override = default;

    // Scene Lifecycle Method Declarations
    void setup() override;
    void tick() override;
    // No getStatus override declared in original header

}; // class OrientationGridScene

} // namespace Scenes