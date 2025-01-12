#include "animations/orientation_demo.h"

void OrientationDemo::init(const AnimParams& params) {
    // Initialize parameters with defaults
    lat_lines = params.getInt("lat_lines", 5);
    lon_lines = params.getInt("lon_lines", 4);
    cycle_time = params.getInt("cycle_time", 1500);
    transition_duration = params.getInt("transition_duration", 200);
    current_line_width = params.getFloat("line_width", 0.14);
    
    // Initialize colors
    target_bg_color = ColorFromPalette(highlightPalette, random8());
    target_line_color = ColorFromPalette(basePalette, random8());
    bg_color = target_bg_color;
    line_color = target_line_color;
    
    pick_new_colors();
}

void OrientationDemo::pick_new_colors() {
    // Store previous target as current
    bg_color_prev = bg_color;
    line_color_prev = line_color;
    bg_color = target_bg_color;
    line_color = target_line_color;
    
    CHSV best_hsv1, best_hsv2;
    float best_score = 0;
    
    // find an acceptable random color pair
    for (int i = 0; i < 16; i++) {
        CHSV hsv1 = rgb2hsv_approximate(ColorFromPalette(uniquePalette, random8()));
        CHSV hsv2 = rgb2hsv_approximate(ColorFromPalette(RainbowStripesColors_p, random8()));
        
        if (get_perceived_brightness(hsv1) > get_perceived_brightness(hsv2)) {
            hsv1.v = qadd8(hsv1.v, 16+random8(32));
            hsv2.v = qsub8(hsv2.v, 16+random8(32));
        } else {
            hsv2.v = qadd8(hsv2.v, 16+random8(32));
            hsv1.v = qsub8(hsv1.v, 16+random8(32));
        }
        
        float contrast = get_contrast_ratio(hsv1, hsv2);
        float hue_dist = get_hue_distance(hsv1, hsv2);
        float score = contrast * (hue_dist / 180.0f);
        
        if (contrast >= 1.5 && hue_dist >= 20.0 && score > best_score) {
            best_score = score;
            best_hsv1 = hsv1;
            best_hsv2 = hsv2;
        }
    }

    // Convert to RGB for final colors
    CRGB bright_color = CHSV(best_hsv1.h, best_hsv1.s, best_hsv1.v);
    CRGB dark_color = CHSV(best_hsv2.h, best_hsv2.s, best_hsv2.v);
    bright_color.fadeLightBy(20);
    dark_color.fadeToBlackBy(50);

    dark_lines = !dark_lines;
    
    if (dark_lines) {
        target_bg_color = bright_color;
        target_line_color = dark_color;
    } else {
        target_bg_color = dark_color;
        target_line_color = bright_color;
    }
}

void OrientationDemo::blend_to_target(float blend_amount) {
    bg_color = blend(bg_color_prev, target_bg_color, blend_amount * 255);
    line_color = blend(line_color_prev, target_line_color, blend_amount * 255);
}

float OrientationDemo::angle_diff(float a1, float a2) {
    float diff = fmod(abs(a1 - a2), TWO_PI);
    return min(diff, TWO_PI - diff);
}

void OrientationDemo::tick() {
    // Check if it's time to start a new transition
    if (++transition_counter >= cycle_time) {
        if (!in_transition) {
            pick_new_colors();
            in_transition = true;
            transition_counter = 0;
        }
    }
    
    // Calculate blend amount during transition
    float rotation_speed = 0.7;
    if (in_transition) {
        if (transition_counter == 0) {
            target_line_width = random(10, 40)/100.0;
        }
        float blend_amount = map(transition_counter, 0, transition_duration, 0, 1000)/1000.0;
        current_line_width += (target_line_width - current_line_width) * blend_amount;
        blend_to_target(blend_amount);
        rotation_speed = 0.7 + sin(blend_amount*PI)*1.4;
        
        if (transition_counter >= transition_duration) {
            in_transition = false;
        }
    }

    // Update rotation angles
    rotation_angle += 0.025 * rotation_speed;
    float spin = rotation_angle;
    float tilt = sin(tilt_speed * 0.002) * 0.5;
    float tumble = rotation_angle * 0.25;
    
    Matrix3d rot_z = AngleAxisd(spin, Vector3d::UnitZ()).matrix();
    Matrix3d rot_x = AngleAxisd(tilt, Vector3d::UnitX()).matrix();
    Matrix3d rot_y = AngleAxisd(tumble, Vector3d::UnitY()).matrix();
    Matrix3d rotation = rot_z * rot_x * rot_y;

    // Draw the grid
    for (int i = 0; i < numLeds(); i++) {
        LED_Point p = points[i];
        Vector3d point(p.x, p.y, p.z);
        Vector3d rotated_point = rotation * point;
        Vector3d ray_dir = rotated_point.normalized();
        
        float a = atan2(ray_dir.y(), ray_dir.x());
        float c = acos(ray_dir.z());
        
        float a_norm = fmod(a + TWO_PI, TWO_PI);
        float c_norm = c;
        
        float lat_spacing = TWO_PI / lat_lines;
        float lon_spacing = PI / lon_lines;
        
        float nearest_lat = round(a_norm / lat_spacing) * lat_spacing;
        float nearest_lon = round(c_norm / lon_spacing) * lon_spacing;
        
        float lat_angle = min(
            abs(angle_diff(a_norm, nearest_lat)),
            abs(angle_diff(a_norm, nearest_lat + lat_spacing))
        );
        
        float lon_angle = min(
            abs(angle_diff(c_norm, nearest_lon)),
            abs(angle_diff(c_norm, nearest_lon + lon_spacing))
        );
        
        float point_dist = point.norm();
        float lat_dist = point_dist * lat_angle;
        float lon_dist = point_dist * lon_angle;
        
        float dist = min(lat_dist, lon_dist);
        float scaled_width = current_line_width * point_dist;

        // set background color
        leds[i] = bg_color;
        leds[i].nscale8(scale8(160, Animation::global_brightness));

        // handle the transition between background and line
        if (dist < scaled_width) {
            float blend_factor = 1.0 - (dist/scaled_width);
            blend_factor = blend_factor * blend_factor * (3 - 2 * blend_factor);

            uint8_t line_intensity = 255 * blend_factor;
            CRGB line = line_color;
            line.nscale8(scale8(line_intensity, Animation::global_brightness));
            leds[i] = blend(leds[i], line, line_intensity);
        }
    }
    
    tilt_speed += 1.0;
}

String OrientationDemo::getStatus() const {
    output.printf("Rotation: %.2f Tilt: %.2f\n", rotation_angle, tilt_speed);
    output.printf("Line width: %.2f -> %.2f\n", current_line_width, target_line_width);
    output.printf("Grid: %d lat x %d lon lines\n", lat_lines, lon_lines);
    
    if (in_transition) {
        output.printf("Transitioning: %d/%d\n", transition_counter, transition_duration);
    }
    
    output.print(getAnsiColorString(bg_color));
    output.printf(" Background (%s)\n", getClosestColorName(bg_color).c_str());
    output.print(getAnsiColorString(line_color));
    output.printf(" Lines (%s)\n", getClosestColorName(line_color).c_str());
    
    return output.get();
} 