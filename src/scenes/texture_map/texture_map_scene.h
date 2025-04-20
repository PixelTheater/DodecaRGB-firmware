#pragma once

#include "PixelTheater/SceneKit.h" // Convenience aliases
#include "textures/texture_data.h" // Includes the generated texture data
#include <vector> // Added for std::vector

namespace Scenes {

class TextureMapScene : public PixelTheater::Scene {
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
    std::vector<const PixelTheater::TextureData*> textures_; // Vector of texture pointers
    size_t current_texture_index_ = 0;      // Index of the currently displayed texture
    float time_since_last_switch_ = 0.0f;    // Timer for switching textures

    // Rotation angle (e.g., around Y-axis)
    float rotation_angle_ = 0.0f;
    uint32_t last_rotation_update_ms_ = 0; // Added: Track time for rotation calc

    // Parameters - Registered in setup() now
};

} // namespace Scenes

// Provide backward‑compatibility alias so existing code using PixelTheater::TextureMapScene keeps compiling
namespace PixelTheater {
    using TextureMapScene = Scenes::TextureMapScene;
} 