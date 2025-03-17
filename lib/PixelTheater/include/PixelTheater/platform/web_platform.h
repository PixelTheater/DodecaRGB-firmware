#pragma once

#include "PixelTheater/core/crgb.h"
#include "PixelTheater/platform/platform.h"
#include "PixelTheater/platform/webgl/camera.h"
#include "PixelTheater/platform/webgl/renderer.h"
#include "PixelTheater/platform/webgl/mesh.h"
#include "PixelTheater/model_def.h"
#include "PixelTheater/platform/webgl/web_model.h"
#include "PixelTheater/platform/webgl/shaders.h"

#include <functional>
#include <vector>
#include <array>
#include <cstdint>
#include <memory>

// Only include WebGL and Emscripten headers in web builds
#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

namespace PixelTheater {
namespace WebGL {

// Add zoom level enum
enum class ZoomLevel {
    CLOSE,
    NORMAL,
    FAR
};

class WebPlatform : public Platform {
public:
    // ============================================
    // Configuration Parameters
    // ============================================
    
    // LED Appearance
    static constexpr float DEFAULT_LED_SIZE = 0.6f;           // Default LED size ratio (1.0 = physically accurate)
    static constexpr float MIN_LED_SIZE_RATIO = 0.2f;         // Minimum LED size ratio
    static constexpr float MAX_LED_SIZE_RATIO = 2.0f;         // Maximum LED size ratio
    static constexpr float PHYSICAL_LED_DIAMETER = 3.8f;      // Physical diameter of each LED in mm
    static constexpr float PHYSICAL_FACE_EDGE = 107.3f;       // Physical edge length of each face in mm
    static constexpr float DEFAULT_ATMOSPHERE_INTENSITY = 1.4f; // Default atmospheric glow intensity
    static constexpr float MIN_ATMOSPHERE_INTENSITY = 0.0f;    // Minimum atmospheric effect
    static constexpr float MAX_ATMOSPHERE_INTENSITY = 3.0f;    // Maximum atmospheric effect
    static constexpr float DEFAULT_LED_SPACING = 5.0f;        // Spacing between LEDs
    static constexpr uint8_t DEFAULT_BRIGHTNESS = 150;        // Initial brightness (0-255)
    
    // Camera Settings
    static constexpr float CAMERA_CLOSE_DISTANCE = 24.0f;      // Close zoom (increased from 12.0f)
    static constexpr float CAMERA_NORMAL_DISTANCE = 32.0f;     // Medium distance (increased from 20.0f)
    static constexpr float CAMERA_FAR_DISTANCE = 55.0f;        // Far zoom (increased from 35.0f)
    static constexpr float CAMERA_FOV_DEGREES = 45.0f;         // Field of view in degrees
    static constexpr float CAMERA_NEAR_PLANE = 0.1f;           // Near clipping plane
    static constexpr float CAMERA_FAR_PLANE = 100.0f;          // Far clipping plane
    
    // Rotation Settings
    static constexpr float ROTATION_SCALE = 0.005f;            // Rotation scale for mouse movement (reduced for smoother turntable)
    static constexpr float MAX_VERTICAL_ROTATION = 1.5f;       // Maximum vertical rotation (about 85 degrees)
    static constexpr float DEFAULT_AUTO_ROTATION_SPEED = 0.5f;  // Default to slow speed
    static constexpr float AUTO_ROTATION_TIME_SCALE = 1.0f;     // No additional scaling needed
    
    // Shader Effects
    static constexpr float COLOR_BRIGHTNESS_BOOST = 1.0f;      // Multiplier for LED color brightness
    static constexpr float MIN_LED_BRIGHTNESS = 0.05f;         // Minimum brightness for visible LEDs
    static constexpr float MAX_DEPTH_FADE = 6.0f;              // Maximum depth for LED visibility fade
    static constexpr float MIN_DEPTH_FADE = 0.4f;              // Minimum depth fade value
    
    explicit WebPlatform();
    ~WebPlatform() override;

    // Prevent copying
    WebPlatform(const WebPlatform&) = delete;
    WebPlatform& operator=(const WebPlatform&) = delete;

    // Model loading
    template<typename ModelDef>
    WebModel createWebModel() {
        WebModel model;
        
        // Metadata
        model.metadata.name = ModelDef::NAME;
        model.metadata.version = ModelDef::VERSION;
        model.metadata.num_leds = ModelDef::LED_COUNT;
        
        // LED positions
        model.leds.positions.reserve(ModelDef::LED_COUNT);
        for (uint16_t i = 0; i < ModelDef::LED_COUNT; i++) {
            // Get coordinates directly from POINTS array
            const auto& point = ModelDef::POINTS[i];
            model.leds.positions.push_back({point.x, point.y, point.z});
        }
        
        // Geometry
        model.geometry.faces.reserve(ModelDef::FACE_COUNT);
        for (uint16_t face = 0; face < ModelDef::FACE_COUNT; face++) {
            WebFace web_face;
            for (uint16_t i = 0; i < 5; i++) {
                web_face.vertices[i] = {
                    ModelDef::FACES[face].vertices[i].x,
                    ModelDef::FACES[face].vertices[i].y,
                    ModelDef::FACES[face].vertices[i].z
                };
            }
            model.geometry.faces.push_back(web_face);
        }
        
        return model;
    }

    // Initialize with a specific model
    template<typename ModelDef>
    void initializeWithModel() {
        WebModel model = createWebModel<ModelDef>();
        initializeFromWebModel(model);
    }

    // Core LED array management
    CRGB* getLEDs() override;
    uint16_t getNumLEDs() const override;

    // Hardware control operations
    void show() override;
    void setBrightness(uint8_t brightness) override;
    uint8_t getBrightness() const;
    void clear() override;

    // Performance settings
    void setMaxRefreshRate(uint8_t fps) override;
    void setDither(uint8_t dither) override;

#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    // WebGL-specific methods - only available in web builds
    void setLEDSize(float size);
    float getLEDSize() const;
    void setAtmosphereIntensity(float intensity);
    float getAtmosphereIntensity() const;
    void setLEDSpacing(float spacing);
    
    // Mesh visualization methods
    void setShowMesh(bool show);
    bool getShowMesh() const;
    void setMeshOpacity(float opacity);
    float getMeshOpacity() const;
    void setShowWireframe(bool show);
    bool getShowWireframe() const;
    
    // Rotation and view control
    void updateRotation(float deltaX, float deltaY);
    void resetRotation();
    void setAutoRotation(bool enabled, float speed = DEFAULT_AUTO_ROTATION_SPEED);
    void setZoomLevel(int zoom_level);
    
    // JavaScript interface methods (called from JS)
    void onCanvasResize(int width, int height);
    void onMouseDown(int x, int y);
    void onMouseMove(int x, int y, bool shift_key);
    void onMouseUp();
    void onMouseWheel(float delta);

    // WebGL resource management
    EMSCRIPTEN_KEEPALIVE void cleanupWebGL();

    // Update a scene parameter in the animation controller
    void updateSceneParameter(const char* param_id, float value);

private:
#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    void initializeFromWebModel(const WebModel& model);
    void initWebGL();
    void updateVertexBuffer();
    void renderFrame();
    void updateAutoRotation();
    void renderMesh(const float* view_matrix, const float* projection_matrix, const float* model_matrix);
    void renderLEDs(const float* view_matrix, const float* projection_matrix, const float* model_matrix);

    // Canvas parameters
    int _canvas_width{800};
    int _canvas_height{600};
    
    // WebGL rendering properties
    float _led_size{DEFAULT_LED_SIZE};
    float _atmosphere_intensity{DEFAULT_ATMOSPHERE_INTENSITY};
    float _led_spacing{DEFAULT_LED_SPACING};
    bool _show_mesh{true};
    float _mesh_opacity{0.3f};
    bool _show_wireframe{true};
    
    // Camera settings
    float _camera_distance{CAMERA_NORMAL_DISTANCE};
    
    // Mouse interaction
    bool _mouse_down{false};
    int _last_mouse_x{0};
    int _last_mouse_y{0};
    
    // Auto-rotation
    bool _auto_rotation{false};
    float _auto_rotation_speed{DEFAULT_AUTO_ROTATION_SPEED};
    
    // Shader resources
    uint32_t _shader_program{0};
    uint32_t _mesh_shader_program{0};
    uint32_t _glow_shader_program{0};
    uint32_t _blur_shader_program{0};
    uint32_t _composite_shader_program{0};
    uint32_t _vertex_buffer{0};
    
    // Performance metrics
    int _frame_count{0};
    double _last_frame_time{0};
    double _last_auto_rotation_time{0};
#endif

    // WebGL components
    std::unique_ptr<WebGLRenderer> _renderer;
    std::unique_ptr<MeshGenerator> _mesh_generator;
    std::unique_ptr<Camera> _camera;

    // Model data
    std::vector<WebVertex> _led_positions;  // Cached LED positions from WebModel

    // LED data
    CRGB* _leds{nullptr};
    uint16_t _num_leds{0};
    uint8_t _brightness{DEFAULT_BRIGHTNESS};
    uint8_t _max_refresh_rate{0};
    uint8_t _dither{0};
};

} // namespace WebGL
} // namespace PixelTheater 

#endif // defined(PLATFORM_WEB) || defined(EMSCRIPTEN) 