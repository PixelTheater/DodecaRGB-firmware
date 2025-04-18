#pragma once

#include "PixelTheater.h"
#include <cmath> // For std::atan2, std::acos, std::fmod, std::min, std::abs, std::round, std::sin, std::cos

// Use shorter Eigen types (Available via ArduinoEigen dependency)
using Vector3f = Eigen::Vector3f;
using Matrix3f = Eigen::Matrix3f;

namespace Scenes {

class OrientationGridScene : public PixelTheater::Scene {
private:
    // --- Parameters (Cached from settings) ---
    int lat_lines_ = 5;
    int lon_lines_ = 4;
    uint16_t cycle_time_frames_ = 1500; // Use frames instead of ms for simplicity with tickCount()
    uint16_t transition_duration_frames_ = 200;
    float target_line_width_ = 0.14f;
    float previous_target_line_width_ = 0.14f; // Store width before transition

    // --- Colors ---
    PixelTheater::CRGB bg_color_;
    PixelTheater::CRGB line_color_;
    PixelTheater::CRGB target_bg_color_;
    PixelTheater::CRGB target_line_color_;
    PixelTheater::CRGB bg_color_prev_;
    PixelTheater::CRGB line_color_prev_;

    // --- State ---
    bool dark_lines_ = true;
    bool in_transition_ = true;
    size_t transition_timer_ = 0; // Frame counter for transitions
    float current_line_width_ = 0.14f;

    // --- Animation ---
    float rotation_angle_ = 0.0f;
    float tilt_speed_ = 0.0f; // Accumulated value, maybe rename?

    // --- Helper Methods ---

    void pickNewColors() {
        // Store previous target as current for blending
        bg_color_prev_ = bg_color_;
        line_color_prev_ = line_color_;
        bg_color_ = target_bg_color_; // Start blend from previous target
        line_color_ = target_line_color_;

        // STORE previous target width BEFORE getting new target from settings
        previous_target_line_width_ = target_line_width_;
        target_line_width_ = settings["line_width"]; // Get potentially updated target width

        PixelTheater::CHSV best_hsv1, best_hsv2; // Store the best pair found based on hue
        PixelTheater::CRGB best_rgb1, best_rgb2; // Store corresponding RGBs (unused currently, but kept for potential future logic)
        bool found_hue_pair = false;
        float best_hue_dist = 0.0f;

        const int MAX_RANDOM_ATTEMPTS = 3; // Try only a few random pairs first
        const uint8_t BRIGHT_THRESHOLD = 200; // Target Value for the bright color
        const uint8_t DARK_THRESHOLD = 60;  // Target Value for the dark color
        const float MIN_HUE_DEG_DIFF = 60.0f; // Minimum hue difference in degrees

        for (int attempt = 0; attempt < MAX_RANDOM_ATTEMPTS; ++attempt) {
            // Pick from two different palettes
            PixelTheater::CRGB current_rgb1 = PixelTheater::colorFromPalette(PixelTheater::Palettes::RainbowStripeColors, random8());
            PixelTheater::CRGB current_rgb2 = PixelTheater::colorFromPalette(PixelTheater::Palettes::PartyColors, random8());

            PixelTheater::CHSV current_hsv1 = PixelTheater::rgb2hsv_approximate(current_rgb1);
            PixelTheater::CHSV current_hsv2 = PixelTheater::rgb2hsv_approximate(current_rgb2);

            // Check for sufficient Hue difference
            float hue_dist = PixelTheater::ColorUtils::get_hue_distance(current_hsv1, current_hsv2);

            if (hue_dist >= MIN_HUE_DEG_DIFF) {
                // Found a pair with good hue separation. Check if it's the best hue separation so far.
                if (!found_hue_pair || hue_dist > best_hue_dist) {
                    found_hue_pair = true;
                    best_hue_dist = hue_dist;
                    best_hsv1 = current_hsv1; // Store this pair's HSV
                    best_hsv2 = current_hsv2;
                    best_rgb1 = current_rgb1; // Store this pair's original RGB
                    best_rgb2 = current_rgb2;

                    // Optional optimization: Check if this pair *also* meets brightness criteria
                    bool check1_bright = best_hsv1.v >= BRIGHT_THRESHOLD && best_hsv2.v <= DARK_THRESHOLD;
                    bool check2_bright = best_hsv2.v >= BRIGHT_THRESHOLD && best_hsv1.v <= DARK_THRESHOLD;
                    if (check1_bright || check2_bright) {
                        break; // Found a perfect pair early, exit loop
                    }
                }
            }
        }

        PixelTheater::CRGB final_bright_rgb;
        PixelTheater::CRGB final_dark_rgb;

        if (found_hue_pair) {
            // We have a pair with good hue distance, now enforce brightness separation
            PixelTheater::CHSV bright_hsv, dark_hsv;

            // Identify initially brighter/darker based on the stored best pair
            if (best_hsv1.v >= best_hsv2.v) {
                bright_hsv = best_hsv1;
                dark_hsv = best_hsv2;
            } else {
                bright_hsv = best_hsv2;
                dark_hsv = best_hsv1;
            }

            // Adjust Value components to meet thresholds
            bright_hsv.v = std::max(bright_hsv.v, BRIGHT_THRESHOLD);
            dark_hsv.v = std::min(dark_hsv.v, DARK_THRESHOLD);

            // Convert the *adjusted* HSV values back to RGB
            PixelTheater::hsv2rgb_rainbow(bright_hsv, final_bright_rgb);
            PixelTheater::hsv2rgb_rainbow(dark_hsv, final_dark_rgb);

        } else {
            // Fallback if no suitable pair (even based on hue) found after attempts
            logWarning("Could not find color pair with sufficient hue distance after %d attempts, using default White/Black.", MAX_RANDOM_ATTEMPTS);
            final_bright_rgb = PixelTheater::CRGB::White;
            final_dark_rgb = PixelTheater::CRGB::Black;
        }

        // NO final nscale8/fade adjustments needed here.

        dark_lines_ = !dark_lines_; // Flip the style

        // Assign final chosen colors to targets
        if (dark_lines_) {
            target_bg_color_ = final_bright_rgb;
            target_line_color_ = final_dark_rgb;
        } else {
            target_bg_color_ = final_dark_rgb;
            target_line_color_ = final_bright_rgb;
        }

        in_transition_ = true;
        transition_timer_ = 0; // Reset timer for the new transition
    }

    void blendToTarget(float blend_amount_0_1) {
        uint8_t blend_u8 = static_cast<uint8_t>(blend_amount_0_1 * 255.0f);
        bg_color_ = PixelTheater::blend(bg_color_prev_, target_bg_color_, blend_u8);
        line_color_ = PixelTheater::blend(line_color_prev_, target_line_color_, blend_u8);
    }

    // Helper to find the shortest angle difference (-PI to PI) -> (0 to PI)
    static inline float angleDiff(float a1, float a2) {
        // Use multiplication by 2.0f to avoid TWO_PI macro collision
        float diff = std::fmod(a1 - a2 + PixelTheater::Constants::PT_PI, (2.0f * PixelTheater::Constants::PT_PI)) - PixelTheater::Constants::PT_PI;
        return std::abs(diff);
    }

public:
    OrientationGridScene() = default;
    ~OrientationGridScene() override = default;

    void config() override {
        set_name("Orientation Grid");
        set_author("Original Author (Ported)");
        set_description("Rotating spherical grid with color transitions");
    }

    void setup() override {
        // Scene::setup(); // REMOVED - Base class call seems unnecessary/problematic

        // Define parameters using snake_case names and descriptions, pass "" for flags
        param("latitude_lines", "count", 2, 20, 5, "", "Number of latitude lines");
        param("longitude_lines", "count", 2, 20, 4, "", "Number of longitude lines");
        param("cycle_time_frames", "count", 100, 5000, 1500, "", "Frames between color changes");
        param("transition_duration_frames", "count", 50, 1000, 200, "", "Frames for color transition");
        param("line_width", "range", 0.02f, 0.5f, 0.14f, "", "Thickness of grid lines");

        // Initialize state from parameters using implicit conversion + cast for uint16_t
        lat_lines_ = settings["latitude_lines"]; // Implicit conversion to int
        lon_lines_ = settings["longitude_lines"]; // Implicit conversion to int
        cycle_time_frames_ = static_cast<uint16_t>(static_cast<int>(settings["cycle_time_frames"])); // Explicit cast via int
        transition_duration_frames_ = static_cast<uint16_t>(static_cast<int>(settings["transition_duration_frames"])); // Explicit cast via int
        target_line_width_ = settings["line_width"]; // Implicit conversion to float
        current_line_width_ = target_line_width_; // Start with target width
        previous_target_line_width_ = target_line_width_; // Initialize previous too

        // Initialize colors
        pickNewColors(); // Pick initial target colors
        // Set current colors directly first time to avoid blending from black
        bg_color_ = target_bg_color_;
        line_color_ = target_line_color_;
        bg_color_prev_ = bg_color_; // Ensure prev matches start
        line_color_prev_ = line_color_; // Ensure prev matches start
        in_transition_ = false; // Start not in transition
        transition_timer_ = cycle_time_frames_; // Set timer so it triggers transition soon
    }

    void tick() override {
        Scene::tick(); // Base class tick (updates tickCount(), etc.)

        // --- Read Parameters (in case they changed) ---
        lat_lines_ = settings["latitude_lines"];
        lon_lines_ = settings["longitude_lines"];
        cycle_time_frames_ = static_cast<uint16_t>(static_cast<int>(settings["cycle_time_frames"]));
        transition_duration_frames_ = static_cast<uint16_t>(static_cast<int>(settings["transition_duration_frames"]));
        // Target line width is read inside pickNewColors when transition starts

        // --- Timing and Transitions ---
        transition_timer_++;
        float rotation_speed = 0.7f; // Base speed

        if (transition_timer_ >= cycle_time_frames_ && !in_transition_) {
            // Time to start a new transition
            pickNewColors(); // This sets in_transition_ = true, resets timer
        }

        if (in_transition_) {
            if (transition_timer_ >= transition_duration_frames_) {
                // End transition
                in_transition_ = false;
                bg_color_ = target_bg_color_; // Ensure final color is set
                line_color_ = target_line_color_;
                current_line_width_ = target_line_width_; // Ensure final width is set
            } else {
                // During transition: blend colors and line width
                float blend_amount_0_1 = PixelTheater::map(
                    static_cast<float>(transition_timer_),
                    0.0f,
                    static_cast<float>(transition_duration_frames_),
                    0.0f,
                    1.0f
                );
                blendToTarget(blend_amount_0_1);
                // Smoothly interpolate line width using standard lerp
                current_line_width_ = previous_target_line_width_ + (target_line_width_ - previous_target_line_width_) * blend_amount_0_1;

                // Vary rotation speed during transition
                rotation_speed = 0.7f + std::sin(blend_amount_0_1 * PixelTheater::Constants::PT_PI) * 1.4f;
            }
        }

        // --- Calculate Rotation ---
        rotation_angle_ += 0.025f * rotation_speed;
        tilt_speed_ += 1.0f; // Simple accumulator for tilt effect
        float spin = rotation_angle_;
        float tilt = std::sin(tilt_speed_ * 0.002f) * 0.5f; // Oscillating tilt
        float tumble = rotation_angle_ * 0.25f; // Slower tumble

        // Manually construct rotation matrices
        float cosSpin = std::cos(spin);
        float sinSpin = std::sin(spin);
        float cosTilt = std::cos(tilt);
        float sinTilt = std::sin(tilt);
        float cosTumble = std::cos(tumble);
        float sinTumble = std::sin(tumble);

        Matrix3f rot_z;
        rot_z << cosSpin, -sinSpin, 0,
                 sinSpin,  cosSpin, 0,
                       0,        0, 1;

        Matrix3f rot_x;
        rot_x << 1,       0,        0,
                 0, cosTilt, -sinTilt,
                 0, sinTilt,  cosTilt;

        Matrix3f rot_y;
        rot_y << cosTumble, 0, sinTumble,
                         0, 1,         0,
                -sinTumble, 0, cosTumble;

        // Combine rotations
        Matrix3f rotation = rot_z * rot_x * rot_y; // Combined rotation

        // --- Render Grid ---
        // Use multiplication by 2.0f to avoid TWO_PI macro collision
        const float lat_spacing = (2.0f * PixelTheater::Constants::PT_PI) / static_cast<float>(lat_lines_);
        const float lon_spacing = PixelTheater::Constants::PT_PI / static_cast<float>(lon_lines_);

        for (size_t i = 0; i < ledCount(); ++i) {
            const PixelTheater::Point& p = model().point(i);
            Vector3f point(p.x(), p.y(), p.z()); // Use float Vector

            Vector3f rotated_point = rotation * point;

            // Avoid normalization if point is at origin
            float norm = rotated_point.norm();
             if (norm < 1e-6f) {
                 leds[i] = bg_color_;
                 continue;
             }
             Vector3f ray_dir = rotated_point / norm; // Manual normalization

            // Convert to spherical coordinates (azimuth, elevation)
            float azimuth = std::atan2(ray_dir.y(), ray_dir.x()); // (-PI to PI)
            float elevation = std::acos(ray_dir.z()); // (0 to PI) - angle from Z+

            // Find angular distance to nearest grid lines
            float nearest_lat_angle = std::round(azimuth / lat_spacing) * lat_spacing;
            float nearest_lon_angle = std::round(elevation / lon_spacing) * lon_spacing;

            float lat_diff = angleDiff(azimuth, nearest_lat_angle);
            float lon_diff = angleDiff(elevation, nearest_lon_angle);

            // Calculate approximate perpendicular distance on sphere surface
            float lat_dist = norm * lat_diff;
            float lon_dist = norm * lon_diff;

            float dist_to_line = std::min(lat_dist, lon_dist);
            float line_thickness_world = current_line_width_ * norm; // Scale line width by distance from center

            // --- Determine Color ---
            if (dist_to_line < line_thickness_world) {
                 // On or near a line - blend towards line_color
                float blend_factor = 1.0f - (dist_to_line / line_thickness_world);
                // Apply ease-in/out curve (smoothstep)
                blend_factor = blend_factor * blend_factor * (3.0f - 2.0f * blend_factor);
                uint8_t blend_u8 = static_cast<uint8_t>(blend_factor * 255.0f);
                leds[i] = PixelTheater::blend(bg_color_, line_color_, blend_u8);
            } else {
                // Background
                leds[i] = bg_color_;
            }
             // NO Global Brightness scaling needed here - Platform handles it.
        }
    }

}; // class OrientationGridScene

} // namespace Scenes