#include "texture_map_scene.h"
// #include "PixelTheater/mapping/coordinate_map.h" // Removed - Assume mapping comes from base Scene/Model
#include <cmath> // For std::fmod, atan2, acos, sqrt, max, min
#include <vector> // Include vector for safety, though already in .h
#include <algorithm>

// Use shorter Eigen types (assuming Eigen headers included via PixelTheater.h)
using Vector3f = Eigen::Vector3f;
// Note: SceneKit already brings common PixelTheater symbols into namespace Scenes.
// Avoid pulling them into the global namespace to prevent collisions with FastLED types.

namespace Scenes {

// Use library constant for PI if available, otherwise define
#ifndef PT_PI
    #ifdef M_PI
        #define PT_PI M_PI
    #else
        #define PT_PI 3.14159265358979323846
    #endif
#else 
    // Use the constant directly from the namespace if defined within PixelTheater includes
    using PixelTheater::Constants::PT_PI;
#endif

// Removed: Constant reference to a single texture
// const TextureData& earthTexture = TEXTURE_EARTH_600_300; 

void TextureMapScene::setup() {
    // Set scene metadata
    set_name("Texture Map");
    set_description("Displays the Earth texture mapped onto the sphere.");
    set_version("2.1");
    set_author("PixelTheater User");

    // Register parameters using constants defined in the header
    param("rotation_speed", "range", -2.0f, 2.0f, DEFAULT_ROTATION_SPEED, "clamp", "Rotation speed (radians/sec)");
    param("brightness", "range", 0.0f, 1.0f, DEFAULT_BRIGHTNESS, "clamp", "Texture brightness multiplier");
    param("switch_interval", "range", 5.0f, 120.0f, DEFAULT_SWITCH_INTERVAL, "clamp", "Texture switch interval (sec)");

    // Populate the texture list (adjust names if generate_props changes them)
    textures_.clear(); // Ensure list is empty before adding
    // Add textures IF they are defined (check preprocessor? or rely on linker error if missing?)
    // For now, assume they exist based on generator output log.
    textures_.push_back(&PixelTheater::TEXTURE_EARTH_600_300);
    //textures_.push_back(&TEXTURE_EYEBALL_600_300);
    //textures_.push_back(&TEXTURE_BASKETBALL_600_300);
    //textures_.push_back(&TEXTURE_MOON_600_300);
    // Add any other textures generated in texture_data.h here

    // Initialization code
    rotation_angle_ = 0.0f;
    current_texture_index_ = 0;
    time_since_last_switch_ = 0.0f;
    last_rotation_update_ms_ = millis(); // Initialize with current time
}

void TextureMapScene::reset() {
    // Reset state
    rotation_angle_ = 0.0f;
    current_texture_index_ = 0; // Start from the first texture
    time_since_last_switch_ = 0.0f; // Reset timer
    last_rotation_update_ms_ = millis(); // Reset rotation timer
}

PixelTheater::CRGB TextureMapScene::getColorFromUV(float u, float v) {
    if (textures_.empty() || current_texture_index_ >= textures_.size()) {
        return PixelTheater::CRGB::Magenta; // Error color if no textures or index out of bounds
    }
    const PixelTheater::TextureData* currentTexture = textures_[current_texture_index_];

    // Clamp/wrap UV coordinates
    u = std::fmod(u, 1.0f);
    if (u < 0.0f) u += 1.0f;
    // v is typically clamped [0, 1]
    v = std::max(0.0f, std::min(1.0f, v));

    // Calculate integer pixel coordinates (simple nearest neighbor for now)
    int x = static_cast<int>(u * currentTexture->width);
    int y = static_cast<int>(v * currentTexture->height);

    // Clamp coordinates to be safe
    x = std::max(0, std::min(x, static_cast<int>(currentTexture->width) - 1));
    y = std::max(0, std::min(y, static_cast<int>(currentTexture->height) - 1));

    // Calculate index in the 1D data array (RGB order)
    int index = (y * currentTexture->width + x) * 3;

    // Ensure index is within bounds (should be, but good practice)
    if (index < 0 || index + 2 >= static_cast<int>(currentTexture->width * currentTexture->height * 3)) {
        return PixelTheater::CRGB::DarkRed; // Different Error color for index bounds issue
    }

    // Retrieve the RGB color from PROGMEM
    uint8_t r = pgm_read_byte(&(currentTexture->data[index + 0]));
    uint8_t g = pgm_read_byte(&(currentTexture->data[index + 1]));
    uint8_t b = pgm_read_byte(&(currentTexture->data[index + 2]));
    
    PixelTheater::CRGB color = PixelTheater::CRGB(r, g, b);

    // Apply brightness scaling
    float brightness_param = settings["brightness"]; // Get brightness value
    uint8_t scale = static_cast<uint8_t>(brightness_param * 255.0f);
    color.r = scale8_video(color.r, scale);
    color.g = scale8_video(color.g, scale);
    color.b = scale8_video(color.b, scale);
    
    return color; // Return scaled color
}

void TextureMapScene::tick() {
    // Recommended: Call base tick first (updates tick_count, deltaTime, etc.)
    PixelTheater::Scene::tick(); 

    // --- Handle Texture Switching --- 
    float switch_interval = this->settings["switch_interval"];
    // Use deltaTime() for intervals tied to frame processing if needed
    time_since_last_switch_ += this->deltaTime(); 
    if (time_since_last_switch_ >= switch_interval) {
        if (!textures_.empty()) {
            current_texture_index_ = (current_texture_index_ + 1) % textures_.size();
        }
        time_since_last_switch_ = 0.0f; // Reset timer
    }
    // --- End Texture Switching ---

    // --- Rotation Update based on millis() --- 
    float speed = this->settings["rotation_speed"]; 
    uint32_t current_millis = millis();
    uint32_t elapsed_ms = current_millis - last_rotation_update_ms_;
    // Only update if time has actually passed to avoid large jumps if millis() wraps or glitches
    if (elapsed_ms > 0 && elapsed_ms < 500) { // Add sanity check for elapsed time (e.g., < 0.5 sec)
      rotation_angle_ += speed * (static_cast<float>(elapsed_ms) / 1000.0f);
      rotation_angle_ = std::fmod(rotation_angle_, 2.0f * PT_PI); // Use PT_PI
    }
    last_rotation_update_ms_ = current_millis;

    // --- DEBUG LOGGING --- Update log to show elapsed_ms
    static uint32_t last_log_time = 0;
    if (current_millis - last_log_time > 1000) { // Log approx every second
        this->logInfo("TextureMap Debug: Speed=%.2f, ElapsedMs=%lu, Angle=%.2f", speed, elapsed_ms, rotation_angle_);
        last_log_time = current_millis;
    }
    // --- END DEBUG LOGGING ---
    
    for (size_t i = 0; i < this->ledCount(); ++i) { // Use ledCount()
        // Get the 3D cartesian coordinates of the LED
        const PixelTheater::Point& p = this->model().point(i); 
        // Convert PixelTheater::Point to Eigen::Vector3f
        Vector3f p_vec(p.x(), p.y(), p.z());

        // Convert cartesian to spherical coordinates (theta, phi)
        float r = p_vec.norm(); // Use Eigen norm()
        if (r < 1e-6f) { // Check against small epsilon
            this->leds[i] = PixelTheater::CRGB::Black; // Center point, map to black
            continue;
        }
        float theta = std::atan2(p_vec.y(), p_vec.x()); // Azimuth
        float phi = std::acos(p_vec.z() / r);          // Inclination from Z+

        // Apply rotation around the Y-axis (changes theta)
        // Simple angle addition might be sufficient if atan2 range is handled
        float rotated_theta = std::fmod(theta + rotation_angle_, 2.0f * PT_PI);
        // Ensure positive range [0, 2*PI)
        // if (rotated_theta < 0) rotated_theta += 2.0f * PT_PI; 
        // Using atan2 for wrapping robustness:
        rotated_theta = std::atan2(std::sin(theta + rotation_angle_), std::cos(theta + rotation_angle_));

        // Map spherical coordinates (longitude, latitude) to texture coordinates (u, v)
        // Equirectangular projection: u relates to longitude (theta), v relates to latitude (phi)
        // Map theta from [-PI, PI] to u [0, 1]
        float u = (rotated_theta + PT_PI) / (2.0f * PT_PI); 
        // Map phi from [0, PI] to v [0, 1]
        float v = phi / PT_PI;                

        // Get color from texture
        this->leds[i] = getColorFromUV(u, v);
    }
}

} // namespace Scenes

// Provide backward‑compatibility alias so existing code using PixelTheater::TextureMapScene keeps compiling
namespace PixelTheater {
    using TextureMapScene = Scenes::TextureMapScene;
} 