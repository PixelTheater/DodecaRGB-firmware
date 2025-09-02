#include "orientation_grid_scene.h"
#include "PixelTheater/core/math_utils.h" // For slerp

// Use shorter Eigen types (Should be included via SceneKit.h -> math.h in header)
// using Vector3f = Eigen::Vector3f;
// using Matrix3f = Eigen::Matrix3f;

// Use namespace alias for convenience
namespace PT = PixelTheater;

namespace Scenes {

// --- Helper Method Implementations ---

void OrientationGridScene::pickNewColors() {
    // Store previous target as current for blending
    bg_color_prev_ = bg_color_;
    line_color_prev_ = line_color_;
    bg_color_ = target_bg_color_; // Start blend from previous target
    line_color_ = target_line_color_;

    // STORE previous target width BEFORE getting new target from settings
    previous_target_line_width_ = target_line_width_;
    target_line_width_ = settings["line_width"]; // Get potentially updated target width

    CHSV best_hsv1, best_hsv2; // Store the best pair found based on hue
    bool found_hue_pair = false;
    float best_hue_dist = 0.0f;

    const int MAX_RANDOM_ATTEMPTS = 5; // Increased attempts slightly
    const uint8_t BRIGHT_THRESHOLD = 220; // Increased for more contrast
    const uint8_t DARK_THRESHOLD = 40;  // Decreased for more contrast
    const uint8_t MIN_SATURATION = 210; // Ensure bright color is not washed out
    const float MIN_HUE_DEG_DIFF = 60.0f;

    for (int attempt = 0; attempt < MAX_RANDOM_ATTEMPTS; ++attempt) {
        // Pick from two different palettes
        CRGB current_rgb1 = colorFromPalette(PT::Palettes::RainbowStripeColors, random8());
        CRGB current_rgb2 = colorFromPalette(PT::Palettes::PartyColors, random8());

        CHSV current_hsv1 = PT::rgb2hsv_approximate(current_rgb1);
        CHSV current_hsv2 = PT::rgb2hsv_approximate(current_rgb2);

        float hue_dist = PT::ColorUtils::get_hue_distance(current_hsv1, current_hsv2);

        if (hue_dist >= MIN_HUE_DEG_DIFF) {
            if (!found_hue_pair || hue_dist > best_hue_dist) {
                found_hue_pair = true;
                best_hue_dist = hue_dist;
                best_hsv1 = current_hsv1; 
                best_hsv2 = current_hsv2;
                // Optional optimization check removed for clarity
            }
        }
    }

    CRGB final_bright_rgb;
    CRGB final_dark_rgb;

    if (found_hue_pair) {
        CHSV bright_hsv, dark_hsv;
        if (best_hsv1.v >= best_hsv2.v) {
            bright_hsv = best_hsv1;
            dark_hsv = best_hsv2;
        } else {
            bright_hsv = best_hsv2;
            dark_hsv = best_hsv1;
        }
        bright_hsv.v = std::max(bright_hsv.v, BRIGHT_THRESHOLD);
        bright_hsv.s = std::max(bright_hsv.s, MIN_SATURATION); // Enforce saturation for bright color
        dark_hsv.v = std::min(dark_hsv.v, DARK_THRESHOLD);

        PT::hsv2rgb_rainbow(bright_hsv, final_bright_rgb);
        PT::hsv2rgb_rainbow(dark_hsv, final_dark_rgb);
    } else {
        logWarning("Could not find color pair with sufficient hue distance after %d attempts, using White/Black.", MAX_RANDOM_ATTEMPTS);
        final_bright_rgb = CRGB::White;
        final_dark_rgb = CRGB::Black;
    }

    dark_lines_ = !dark_lines_; 

    if (dark_lines_) {
        target_bg_color_ = final_bright_rgb;
        target_line_color_ = final_dark_rgb;
    } else {
        target_bg_color_ = final_dark_rgb;
        target_line_color_ = final_bright_rgb;
    }

    in_transition_ = true;
    transition_timer_ = 0; 
}

void OrientationGridScene::blendToTarget(float blend_amount_0_1) {
    uint8_t blend_u8 = static_cast<uint8_t>(blend_amount_0_1 * 255.0f);
    bg_color_ = ::Scenes::blend(bg_color_prev_, target_bg_color_, blend_u8); 
    line_color_ = ::Scenes::blend(line_color_prev_, target_line_color_, blend_u8);
}

// Use M_PI from <cmath>
/* static */ float OrientationGridScene::angleDiff(float a1, float a2) {
    // float diff = std::fmod(a1 - a2 + PT::PT_PI, (2.0f * PT::PT_PI)) - PT::PT_PI;
    float diff = std::fmod(a1 - a2 + M_PI, (2.0f * M_PI)) - M_PI;
    return std::abs(diff);
}

// --- Public Method Implementations ---

void OrientationGridScene::setup() {
    // Optionally set metadata here too/instead
    set_name("Orientation Grid"); 
    set_author("PixelTheater Port");
    set_description("Rotating spherical grid with color transitions");
    set_version("2.1");

    // --- MOVE Parameter Definitions Here ---
    param("latitude_lines", "count", 2, 20, 5, "", "Number of latitude lines");
    param("longitude_lines", "count", 2, 20, 4, "", "Number of longitude lines");
    param("cycle_time_frames", "count", 100, 5000, 1500, "", "Frames between color changes");
    param("transition_duration_frames", "count", 50, 1000, 200, "", "Frames for color transition");
    param("line_width", "range", 0.02f, 0.5f, 0.14f, "", "Thickness of grid lines");
    
    // --- IMU Parameters ---
    param("use_imu", "switch", false, "", "Control orientation with IMU");
    param("sensitivity", "range", 1.0f, 10.0f, 2.5f, "", "Movement sensitivity");
    param("stiffness", "ratio", 0.01f, 1.0f, 0.2f, "", "Spring stiffness for IMU smoothing");
    param("damping", "ratio", 0.01f, 1.0f, 0.25f, "", "Damping for IMU smoothing");
    param("imu_quat_real", "signed_ratio", 1.0f, "hidden", "IMU Quaternion Real");
    param("imu_quat_i", "signed_ratio", 0.0f, "hidden", "IMU Quaternion I");
    param("imu_quat_j", "signed_ratio", 0.0f, "hidden", "IMU Quaternion J");
    param("imu_quat_k", "signed_ratio", 0.0f, "hidden", "IMU Quaternion K");

    // --- End MOVE ---

    // Read initial parameter values
    lat_lines_ = settings["latitude_lines"]; 
    lon_lines_ = settings["longitude_lines"]; 
    cycle_time_frames_ = static_cast<uint16_t>(static_cast<int>(settings["cycle_time_frames"])); 
    transition_duration_frames_ = static_cast<uint16_t>(static_cast<int>(settings["transition_duration_frames"])); 
    target_line_width_ = settings["line_width"]; 
    current_line_width_ = target_line_width_; 
    previous_target_line_width_ = target_line_width_; 

    // Initialize colors
    pickNewColors(); 
    bg_color_ = target_bg_color_;
    line_color_ = target_line_color_;
    bg_color_prev_ = bg_color_;
    line_color_prev_ = line_color_;
    in_transition_ = false; 
    transition_timer_ = cycle_time_frames_; 
    // Reset animation state too
    rotation_angle_ = 0.0f;
    tilt_speed_ = 0.0f;
    display_orientation_.setIdentity();
    angular_velocity_.setZero();
}

void OrientationGridScene::tick() {
    Scene::tick(); 

    // --- Read Parameters ---
    // (We read these here to allow them to be changed live)
    lat_lines_ = settings["latitude_lines"];
    lon_lines_ = settings["longitude_lines"];
    cycle_time_frames_ = static_cast<uint16_t>(static_cast<int>(settings["cycle_time_frames"]));
    transition_duration_frames_ = static_cast<uint16_t>(static_cast<int>(settings["transition_duration_frames"]));
    
    // Check if IMU control is enabled
    bool use_imu_setting = settings["use_imu"];

    // --- Timing and Transitions ---
    transition_timer_++;
    float rotation_speed = 0.7f; 

    if (transition_timer_ >= cycle_time_frames_ && !in_transition_) {
        pickNewColors(); 
    }

    if (in_transition_) {
        if (transition_timer_ >= transition_duration_frames_) {
            in_transition_ = false;
            bg_color_ = target_bg_color_; 
            line_color_ = target_line_color_;
            current_line_width_ = target_line_width_; 
        } else {
            // Use map() function (aliased in SceneKit)
            // float blend_amount_0_1 = mapRange(
            float blend_amount_0_1 = map(
                static_cast<float>(transition_timer_),
                0.0f,
                static_cast<float>(transition_duration_frames_),
                0.0f,
                1.0f
            );
            float eased_blend_amount = PT::Easing::inOutSineF(blend_amount_0_1); // Assuming Easing is available
            blendToTarget(eased_blend_amount);
            current_line_width_ = previous_target_line_width_ + (target_line_width_ - previous_target_line_width_) * eased_blend_amount;
            // Use M_PI from <cmath>
            // rotation_speed = 0.7f + std::sin(blend_amount_0_1 * PT::PT_PI) * 1.4f;
            rotation_speed = 0.7f + std::sin(blend_amount_0_1 * M_PI) * 1.4f;
        }
    }

    // --- Calculate Rotation ---
    if (use_imu_setting) {
        // --- IMU-based Rotation ---
        if (!imu_enabled_) {
            // First frame switching to IMU mode, snap to current orientation
            imu_enabled_ = true;
            target_orientation_.w() = settings["imu_quat_real"];
            target_orientation_.x() = settings["imu_quat_i"];
            target_orientation_.y() = settings["imu_quat_j"];
            target_orientation_.z() = settings["imu_quat_k"];
            target_orientation_.normalize();
            // Apply conjugation here to fix reversed rotation from the start
            display_orientation_ = target_orientation_.conjugate(); 
            angular_velocity_.setZero();
        }

        // Update target orientation from IMU
        target_orientation_.w() = settings["imu_quat_real"];
        target_orientation_.x() = settings["imu_quat_i"];
        target_orientation_.y() = settings["imu_quat_j"];
        target_orientation_.z() = settings["imu_quat_k"];
        target_orientation_.normalize();

        // The IMU provides the device's orientation. We conjugate it to get a rotation
        // that moves the *world* around the *device*, which feels more intuitive for this scene.
        Eigen::Quaternionf corrected_target = target_orientation_.conjugate();

        // --- Physics-based Smoothing ---
        float dt = deltaTime();
        if (dt > 0) {
            float stiffness = settings["stiffness"];
            float damping = settings["damping"];
            float sensitivity = settings["sensitivity"];

            // Calculate the rotational difference (error)
            Eigen::Quaternionf diff = corrected_target * display_orientation_.inverse();
            
            // Convert to axis-angle to get a torque vector
            Eigen::AngleAxisf aa(diff);
            Eigen::Vector3f torque = aa.axis() * (aa.angle() * sensitivity);

            // Apply spring force (torque) to velocity
            angular_velocity_ += torque * stiffness;

            // Apply damping
            angular_velocity_ *= (1.0f - damping);

            // Apply angular velocity to the display orientation
            if (angular_velocity_.norm() > 1e-6) {
                Eigen::AngleAxisf rotation_step(angular_velocity_.norm() * dt, angular_velocity_.normalized());
                display_orientation_ = Eigen::Quaternionf(rotation_step) * display_orientation_;
                display_orientation_.normalize();
            }
        }
        
        rotation_matrix_ = display_orientation_.toRotationMatrix();

    } else {
        // --- Automatic Animated Rotation ---
        if (imu_enabled_) {
            // First frame switching OFF IMU mode, reset animation state
            // to prevent a sudden jump from IMU orientation.
            rotation_angle_ = 0.0f; // Or try to match current yaw from quat? For now, just reset.
            tilt_speed_ = 0.0f;
            imu_enabled_ = false;
            display_orientation_ = Eigen::AngleAxisf(0, Eigen::Vector3f::UnitZ());
        }
        rotation_angle_ += 0.025f * rotation_speed;
        tilt_speed_ += 1.0f; 
        float spin = rotation_angle_;
        float tilt = std::sin(tilt_speed_ * 0.002f) * 0.5f; 
        float tumble = rotation_angle_ * 0.25f; 

        float cosSpin = std::cos(spin), sinSpin = std::sin(spin);
        float cosTilt = std::cos(tilt), sinTilt = std::sin(tilt);
        float cosTumble = std::cos(tumble), sinTumble = std::sin(tumble);

        Matrix3f Rz, Ry, Rx;
        Rz << cosSpin, -sinSpin, 0, sinSpin, cosSpin, 0, 0, 0, 1;
        Rx << 1, 0, 0, 0, cosTilt, -sinTilt, 0, sinTilt, cosTilt;
        Ry << cosTumble, 0, sinTumble, 0, 1, 0, -sinTumble, 0, cosTumble;
        rotation_matrix_ = Rz * Rx * Ry;
        // When switching back to auto, update display_orientation to match
        display_orientation_ = rotation_matrix_;
    }

    // --- Render Grid ---
    const float lat_spacing = (2.0f * M_PI) / static_cast<float>(lat_lines_);
    const float lon_spacing = M_PI / static_cast<float>(lon_lines_);

    for (size_t i = 0; i < ledCount(); ++i) {
        const PT::Point& p = model().point(i);
        Vector3f point(p.x(), p.y(), p.z()); 
        Vector3f rotated_point = rotation_matrix_ * point;
        float norm = rotated_point.norm();
         if (norm < 1e-6f) {
             leds[i] = bg_color_;
             continue;
         }
         Vector3f ray_dir = rotated_point / norm; 

        float azimuth = std::atan2(ray_dir.y(), ray_dir.x()); 
        float elevation = std::acos(ray_dir.z()); 

        float nearest_lat_angle = std::round(azimuth / lat_spacing) * lat_spacing;
        float nearest_lon_angle = std::round(elevation / lon_spacing) * lon_spacing;

        float lat_diff = angleDiff(azimuth, nearest_lat_angle);
        float lon_diff = angleDiff(elevation, nearest_lon_angle);

        float lat_dist = norm * lat_diff;
        float lon_dist = norm * lon_diff;

        float dist_to_line = std::min(lat_dist, lon_dist);
        float line_thickness_world = current_line_width_ * norm; 

        if (dist_to_line < line_thickness_world) {
            float blend_factor = 1.0f - (dist_to_line / line_thickness_world);
            blend_factor = blend_factor * blend_factor * (3.0f - 2.0f * blend_factor);
            uint8_t blend_u8 = static_cast<uint8_t>(blend_factor * 255.0f);
            leds[i] = ::Scenes::blend(bg_color_, line_color_, blend_u8);
        } else {
            leds[i] = bg_color_;
        }
    }
}

} // namespace Scenes 