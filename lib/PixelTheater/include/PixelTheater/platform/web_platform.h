#pragma once

#include "PixelTheater/core/crgb.h"
#include "PixelTheater/platform/platform.h"

// Only include WebGL and Emscripten headers in web builds
#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <vector>
#include <functional>
#include <cmath>

namespace PixelTheater {

// Forward declarations and typedefs for non-web builds
#if !defined(PLATFORM_WEB) && !defined(EMSCRIPTEN)
// Just enough to make the header compile, implementation will be empty
typedef unsigned int GLuint;
typedef int GLint;
#endif

// Define a type for the coordinate provider callback
using CoordinateProviderCallback = std::function<void(uint16_t index, float& x, float& y, float& z)>;

// Define preset view types
enum class PresetView {
    SIDE = 0,    // Side view
    TOP = 1,     // Top-down view
    ANGLE = 2    // Angled view (45 degrees)
};

// Define zoom levels
enum class ZoomLevel {
    CLOSE = 0,
    NORMAL = 1,
    FAR = 2
};

class WebPlatform : public Platform {
public:
    // ============================================
    // Configuration Parameters
    // ============================================
    
    // LED Appearance
    static constexpr float DEFAULT_LED_SIZE = 25.0f;           // Default LED size in pixels
    static constexpr float DEFAULT_GLOW_INTENSITY = 1.2f;      // Default glow/bloom effect intensity
    static constexpr float DEFAULT_LED_SPACING = 5.0f;         // Spacing between LEDs
    static constexpr uint8_t DEFAULT_BRIGHTNESS = 128;         // Initial brightness (0-255)
    
    // Camera Settings
    static constexpr float CAMERA_CLOSE_DISTANCE = 2.0f;       // Camera distance for close zoom
    static constexpr float CAMERA_NORMAL_DISTANCE = 3.0f;      // Camera distance for normal zoom
    static constexpr float CAMERA_FAR_DISTANCE = 4.0f;         // Camera distance for far zoom
    static constexpr float CAMERA_FOV_DEGREES = 30.0f;         // Field of view in degrees
    static constexpr float CAMERA_NEAR_PLANE = 0.1f;           // Near clipping plane
    static constexpr float CAMERA_FAR_PLANE = 80.0f;          // Far clipping plane
    
    // Rotation Settings
    static constexpr float ROTATION_SCALE = 0.0017f;           // Scale factor for mouse/touch rotation
    static constexpr float MAX_VERTICAL_ROTATION = 1.5f;       // Maximum vertical rotation (about 85 degrees)
    static constexpr float DEFAULT_AUTO_ROTATION_SPEED = 1.0f; // Default auto-rotation speed
    static constexpr float AUTO_ROTATION_TIME_SCALE = 0.17f;   // Time scaling for auto-rotation
    
    // Shader Effects
    static constexpr float COLOR_BRIGHTNESS_BOOST = 4.0f;      // Multiplier for LED color brightness
    static constexpr float MIN_LED_BRIGHTNESS = 0.05f;         // Minimum brightness for visible LEDs
    static constexpr float MAX_DEPTH_FADE = 8.0f;              // Maximum depth for LED visibility fade
    static constexpr float MIN_DEPTH_FADE = 0.3f;              // Minimum depth fade value
    
    // View Presets (in radians)
    static constexpr float TOP_VIEW_X_ROTATION = -1.57f;       // Top view X rotation (about -90 degrees)
    static constexpr float ANGLE_VIEW_X_ROTATION = -0.6f;      // Angle view X rotation (about -35 degrees)
    static constexpr float ANGLE_VIEW_Y_ROTATION = 0.6f;       // Angle view Y rotation (about 35 degrees)
    
    explicit WebPlatform(uint16_t num_leds);
    ~WebPlatform() override;

    // Prevent copying
    WebPlatform(const WebPlatform&) = delete;
    WebPlatform& operator=(const WebPlatform&) = delete;

    // Core LED array management
    CRGB* getLEDs() override;
    uint16_t getNumLEDs() const override;

    // Hardware control operations
    void show() override;
    void setBrightness(uint8_t brightness) override;
    void clear() override;

    // Performance settings
    void setMaxRefreshRate(uint8_t fps) override;
    void setDither(uint8_t dither) override;

#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    // WebGL-specific methods - only available in web builds
    void setLEDSize(float size);
    float getLEDSize() const;
    void setGlowIntensity(float intensity);
    float getGlowIntensity() const;
    void setLEDSpacing(float spacing);
    void setLEDArrangement(const float* positions, uint16_t count);
    
    // Set a callback to provide 3D coordinates for each LED
    void setCoordinateProvider(CoordinateProviderCallback callback) {
        _coordinate_provider = callback;
    }
    
    // Legacy rotation methods (kept for compatibility)
    void updateRotation(float deltaX, float deltaY) {
        // Convert pixel movement to radians (scale down for smoother rotation)
        
        // Apply rotation directly to model (not camera)
        // For model rotation, we need to invert the direction compared to camera rotation
        _rotation_y -= deltaX * ROTATION_SCALE; // Inverted for natural model rotation
        _rotation_x -= deltaY * ROTATION_SCALE; // Inverted for natural model rotation
        
        // Limit vertical rotation to avoid flipping
        if (_rotation_x > MAX_VERTICAL_ROTATION) _rotation_x = MAX_VERTICAL_ROTATION;
        if (_rotation_x < -MAX_VERTICAL_ROTATION) _rotation_x = -MAX_VERTICAL_ROTATION;
        
        // Normalize horizontal rotation to keep it within reasonable bounds
        while (_rotation_y > 6.28318f) _rotation_y -= 6.28318f; // 2*PI
        while (_rotation_y < -6.28318f) _rotation_y += 6.28318f;
    }
    
    void resetRotation() {
        _rotation_x = 0.0f;
        _rotation_y = 0.0f;
        _auto_rotation = false;
    }
    
    // New rotation and view methods
    void setAutoRotation(bool enabled, float speed) {
        _auto_rotation = enabled;
        _auto_rotation_speed = speed;
    }
    
    void setPresetView(int preset_index) {
        // Don't reset auto-rotation when changing views
        // Store current auto-rotation state
        bool was_auto_rotating = _auto_rotation;
        float rotation_speed = _auto_rotation_speed;
        
        // Set rotation based on preset
        switch (static_cast<PresetView>(preset_index)) {
            case PresetView::SIDE:
                _rotation_x = 0.0f;
                _rotation_y = 0.0f;
                break;
            case PresetView::TOP:
                _rotation_x = TOP_VIEW_X_ROTATION; // About 90 degrees (π/2) - look directly from above
                _rotation_y = 0.0f;
                break;
            case PresetView::ANGLE:
                _rotation_x = ANGLE_VIEW_X_ROTATION; // About 35 degrees
                _rotation_y = ANGLE_VIEW_Y_ROTATION; // About 35 degrees
                break;
            default:
                // Default to side view
                _rotation_x = 0.0f;
                _rotation_y = 0.0f;
                break;
        }
        
        // Restore auto-rotation if it was enabled
        if (was_auto_rotating) {
            _auto_rotation = true;
            _auto_rotation_speed = rotation_speed;
        }
    }
    
    void setZoomLevel(int zoom_level) {
        switch (static_cast<ZoomLevel>(zoom_level)) {
            case ZoomLevel::CLOSE:
                _camera_distance = CAMERA_CLOSE_DISTANCE;
                break;
            case ZoomLevel::NORMAL:
                _camera_distance = CAMERA_NORMAL_DISTANCE;
                break;
            case ZoomLevel::FAR:
                _camera_distance = CAMERA_FAR_DISTANCE; // Reduced from 4.5f to maintain visibility
                break;
            default:
                _camera_distance = CAMERA_NORMAL_DISTANCE; // Default to normal
                break;
        }
    }
    
    // Method to update auto-rotation (called from show())
    void updateAutoRotation() {
        if (_auto_rotation) {
            // Get the time since last frame to make rotation speed consistent
            static double last_time = emscripten_get_now() / 1000.0;
            double current_time = emscripten_get_now() / 1000.0;
            double delta_time = current_time - last_time;
            last_time = current_time;
            
            // Calculate rotation amount based on speed and time
            float rotationAmount = _auto_rotation_speed * delta_time * AUTO_ROTATION_TIME_SCALE;
            
            // Apply rotation based on current view
            if (std::abs(_rotation_x + 1.57f) < 0.1f) {
                // We're in top view (looking down at -90 degrees)
                // For top view, we need to rotate around the Y axis in world space
                // but this appears as a rotation in the XZ plane from our perspective
                _rotation_y += rotationAmount;
            } else {
                // For side and angle views, rotate around Y axis as normal
                _rotation_y += rotationAmount;
            }
            
            // Keep rotation within 0 to 2π range
            while (_rotation_y > 6.28318f) _rotation_y -= 6.28318f; // 2*PI
            while (_rotation_y < 0) _rotation_y += 6.28318f;
        }
    }

private:
    bool initWebGL();
    void updateVertexBuffer();
    void createViewMatrix(float* view_matrix);

    CRGB* _leds{nullptr};
    uint16_t _num_leds{0};
    uint8_t _brightness{255};
    uint8_t _max_refresh_rate{0};
    uint8_t _dither{0};

    // WebGL resources
    bool _gl_initialized{false};
    GLuint _vbo{0};
    GLuint _vao{0};
    GLuint _shader_program{0};

    // WebGL rendering properties
    float _led_size{DEFAULT_LED_SIZE};
    float _glow_intensity{DEFAULT_GLOW_INTENSITY};
    float _led_spacing{DEFAULT_LED_SPACING};
    float* _led_positions{nullptr};
    bool _custom_arrangement{false};
    
    // Rotation state
    float _rotation_x = 0.0f;
    float _rotation_y = 0.0f;
    
    // Auto-rotation state
    bool _auto_rotation = false;
    float _auto_rotation_speed = DEFAULT_AUTO_ROTATION_SPEED;
    
    // Camera settings
    float _camera_distance = CAMERA_NORMAL_DISTANCE;
    
    // Coordinate provider callback
    CoordinateProviderCallback _coordinate_provider{nullptr};
    
    // Uniform locations
    int _projectionLoc{-1};
    int _viewLoc{-1};
    int _colorLoc{-1};
    int _pointSizeLoc{-1};
    int _timeLoc{-1};

    // WebGL initialization and rendering methods
    void renderLEDs();
    void cleanupWebGL();
    GLuint compileShader(unsigned int type, const char* source);
#else
    // Empty stub methods for non-web builds
    void setLEDSize(float size) {}
    float getLEDSize() const { return DEFAULT_LED_SIZE; }
    void setGlowIntensity(float intensity) {}
    float getGlowIntensity() const { return DEFAULT_GLOW_INTENSITY; }
    void setLEDSpacing(float spacing) {}
    void setLEDArrangement(const float* positions, uint16_t count) {}
    void setCoordinateProvider(CoordinateProviderCallback callback) {}
    void updateRotation(float deltaX, float deltaY) {}
    void resetRotation() {}
    void setAutoRotation(bool enabled, float speed) {}
    void setPresetView(int preset_index) {}
    void setZoomLevel(int zoom_level) {}
    void updateAutoRotation() {}

private:
    CRGB* _leds{nullptr};
    uint16_t _num_leds{0};
    uint8_t _brightness{255};
    uint8_t _max_refresh_rate{0};
    uint8_t _dither{0};
#endif
};

} // namespace PixelTheater 