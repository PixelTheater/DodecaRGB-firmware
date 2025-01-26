#pragma once
#include "animation.h"
#include "points.h"
#include "palettes.h"


using Eigen::Vector3d;
using Eigen::Matrix3d;
using Eigen::AngleAxisd;

class OrientationDemo : public Animation {
private:
    // Colors
    CRGB bg_color;
    CRGB line_color;
    CRGB target_bg_color;
    CRGB target_line_color;
    CRGB bg_color_prev;
    CRGB line_color_prev;
    
    // State
    bool dark_lines;
    bool in_transition;
    uint16_t transition_counter;
    float tilt_speed;
    float rotation_angle;
    
    // Grid configuration
    float current_line_width;
    float target_line_width;
    int lat_lines;
    int lon_lines;
    
    // Timing constants
    uint16_t cycle_time;
    uint16_t transition_duration;
    
    // Helper methods
    void pick_new_colors();
    void blend_to_target(float blend_amount);
    float angle_diff(float a1, float a2);

public:
    OrientationDemo() : 
        bg_color_prev(CRGB::Black),
        line_color_prev(CRGB::Black),
        dark_lines(true),
        in_transition(true),
        transition_counter(900),
        tilt_speed(0.0),
        rotation_angle(0.0),
        current_line_width(0.14),
        target_line_width(0.14) {}
        
    void init(const AnimParams& params) override;
    void tick() override;
    String getStatus() const override;
    const char* getName() const override { return "orientation_demo"; }
}; 