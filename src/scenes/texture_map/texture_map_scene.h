#pragma once

#include "PixelTheater.h" // Corrected include path
#include "texture_data.h" // Includes the generated texture data
#include <vector> // Added for std::vector

namespace PixelTheater {

class TextureMapScene : public Scene {
public:
    // Constructor: Use default name or allow override
    TextureMapScene() = default;

    void setup() override;
    void reset() override;
    void tick() override;

private:
    // Helper function to get color from the current texture's coordinates (u, v)
    CRGB getColorFromUV(float u, float v);

    // Texture management
    std::vector<const TextureData*> textures_; // Vector to hold pointers to available textures
    size_t current_texture_index_ = 0;      // Index of the currently displayed texture
    float time_since_last_switch_ = 0.0f;    // Timer for switching textures

    // Rotation angle (e.g., around Y-axis)
    float rotation_angle_ = 0.0f;
    uint32_t last_rotation_update_ms_ = 0; // Added: Track time for rotation calc

    // Parameters - Registered in setup() now
};

} // namespace PixelTheater 