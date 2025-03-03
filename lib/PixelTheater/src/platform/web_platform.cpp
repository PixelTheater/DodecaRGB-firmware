#include "PixelTheater/platform/web_platform.h"
#include "PixelTheater/platform/webgl/renderer.h"
#include "PixelTheater/platform/webgl/mesh.h"
#include "PixelTheater/platform/webgl/camera.h"
#include "PixelTheater/platform/webgl/shaders.h"
#include "PixelTheater/platform/webgl/math.h"
#include <emscripten.h>
#include <cmath>
#include <cstdio>
#include <algorithm>

#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
#include <GLES3/gl3.h>
#include <emscripten/html5.h>
#endif

// External C functions for JavaScript interop
extern "C" {
    EMSCRIPTEN_KEEPALIVE int get_canvas_width();
    EMSCRIPTEN_KEEPALIVE int get_canvas_height();
    EMSCRIPTEN_KEEPALIVE double get_current_time();
    EMSCRIPTEN_KEEPALIVE void update_ui_fps(int fps);
    EMSCRIPTEN_KEEPALIVE void update_ui_brightness(float brightness);
}

namespace PixelTheater {

// Constructor / Destructor
WebPlatform::WebPlatform(uint16_t num_leds) : 
    _leds(new CRGB[num_leds]),
    _num_leds(num_leds),
    _brightness(DEFAULT_BRIGHTNESS),
    _max_refresh_rate(0),
    _dither(0),
    _shader_program(0),
    _mesh_shader_program(0),
    _glow_shader_program(0),
    _blur_shader_program(0),
    _composite_shader_program(0),
    _vertex_buffer(0),
    _atmosphere_intensity(DEFAULT_ATMOSPHERE_INTENSITY),
    _led_size(DEFAULT_LED_SIZE),
    _show_mesh(true),
    _mesh_opacity(0.3f),
    _camera_distance(CAMERA_NORMAL_DISTANCE),
    _mouse_down(false),
    _last_mouse_x(0),
    _last_mouse_y(0),
    _auto_rotation(true),
    _auto_rotation_speed(DEFAULT_AUTO_ROTATION_SPEED),
    _frame_count(0),
    _last_frame_time(0),
    _last_auto_rotation_time(get_current_time()),
    _coordinate_provider(nullptr),
    _mesh_generator([](uint16_t index, float& x, float& y, float& z) {
        // Default to a circle layout
        float angle = 2.0f * M_PI * static_cast<float>(index) / 100.0f;
        x = cos(angle);
        y = sin(angle);
        z = 0.0f;
    }, num_leds)
{
    // Initialize LEDs to black
    for (uint16_t i = 0; i < num_leds; i++) {
        _leds[i] = CRGB(0, 0, 0);
    }
    
#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    // Generate the mesh with default coordinates
    _mesh_generator.generateDodecahedronMesh();
#endif
}

WebPlatform::~WebPlatform() {
    delete[] _leds;
}

// Core LED array management
CRGB* WebPlatform::getLEDs() {
    return _leds;
}

uint16_t WebPlatform::getNumLEDs() const {
    return _num_leds;
}

// Hardware control operations
void WebPlatform::show() {
#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    // Initialize WebGL if not already done
    if (_shader_program == 0) {
        // Initialize WebGL renderer
        try {
            // Get canvas dimensions
            _canvas_width = get_canvas_width();
            _canvas_height = get_canvas_height();
            
            // Initialize WebGL renderer
            if (!_renderer.initialize(_canvas_width, _canvas_height)) {
                fprintf(stderr, "Failed to initialize WebGL renderer\n");
                return;
            }
            
            // Create shader programs
            _shader_program = _renderer.createShaderProgram(
                vertex_shader_source, fragment_shader_source);
            
            _mesh_shader_program = _renderer.createShaderProgram(
                mesh_vertex_shader_source, mesh_fragment_shader_source);
            
            // Create glow shader programs
            _glow_shader_program = _renderer.createShaderProgram(
                glow_vertex_shader_source, glow_fragment_shader_source);
            
            _blur_shader_program = _renderer.createShaderProgram(
                blur_vertex_shader_source, blur_fragment_shader_source);
            
            _composite_shader_program = _renderer.createShaderProgram(
                quad_vertex_shader_source, composite_fragment_shader_source);
            
            // Initialize camera
            _camera = Camera();
            
            printf("WebGL initialized successfully\n");
        } catch (const std::exception& e) {
            fprintf(stderr, "WebGL initialization error: %s\n", e.what());
            return;
        }
    }
    
    // Update vertex buffer with current LED colors
    updateVertexBuffer();
    
    // Render the frame
    renderFrame();
    
    // Update FPS counter
    double current_time = get_current_time();
    _frame_count++;
    
    if (current_time - _last_frame_time >= 1.0) {
        int fps = static_cast<int>(_frame_count / (current_time - _last_frame_time));
        update_ui_fps(fps);
        _frame_count = 0;
        _last_frame_time = current_time;
    }
    
    // Update auto-rotation if enabled
    updateAutoRotation();
#endif
}

void WebPlatform::setBrightness(uint8_t brightness) {
    _brightness = brightness;
#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    // Update UI if available
    update_ui_brightness(static_cast<float>(brightness) / 255.0f);
#endif
}

uint8_t WebPlatform::getBrightness() const {
    return _brightness;
}

void WebPlatform::clear() {
    for (uint16_t i = 0; i < _num_leds; i++) {
        _leds[i] = CRGB(0, 0, 0);
    }
}

// Performance settings
void WebPlatform::setMaxRefreshRate(uint8_t fps) {
    _max_refresh_rate = fps;
}

void WebPlatform::setDither(uint8_t dither) {
    _dither = dither;
}

#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
// WebGL-specific methods
void WebPlatform::setLEDSize(float size) {
    // Clamp size between min and max values
    _led_size = std::max(MIN_LED_SIZE_RATIO, std::min(MAX_LED_SIZE_RATIO, size));
}

float WebPlatform::getLEDSize() const {
    return _led_size;
}

void WebPlatform::setAtmosphereIntensity(float intensity) {
    _atmosphere_intensity = std::clamp(intensity, MIN_ATMOSPHERE_INTENSITY, MAX_ATMOSPHERE_INTENSITY);
}

float WebPlatform::getAtmosphereIntensity() const {
    return _atmosphere_intensity;
}

void WebPlatform::setLEDSpacing(float spacing) {
    _led_spacing = spacing;
}

// Mesh visualization methods
void WebPlatform::setShowMesh(bool show) {
    _show_mesh = show;
}

bool WebPlatform::getShowMesh() const {
    return _show_mesh;
}

void WebPlatform::setMeshOpacity(float opacity) {
    // Clamp opacity between 0 and 1
    _mesh_opacity = std::max(0.0f, std::min(1.0f, opacity));
}

float WebPlatform::getMeshOpacity() const {
    return _mesh_opacity;
}

// Set a callback to provide 3D coordinates for each LED
void WebPlatform::setCoordinateProvider(std::function<void(uint16_t, float&, float&, float&)> callback) {
    _coordinate_provider = callback;
    
    // If we already have a renderer initialized, update the mesh
    if (_coordinate_provider) {
        _mesh_generator = MeshGenerator(_coordinate_provider, _num_leds);
        _mesh_generator.generateDodecahedronMesh();
    }
}

// Helper methods
void WebPlatform::updateVertexBuffer() {
    // Structure for each vertex (position + color)
    struct Vertex {
        float x, y, z;  // 3D coordinates
        float r, g, b;  // Color
    };
    
    // Create vertex data
    std::vector<Vertex> vertices(_num_leds);
    
    // Scale factor to fit LEDs in scene
    constexpr float POSITION_SCALE = 0.03f;
    constexpr float Z_CORRECTION = 1.0f;
    
    // Use coordinate provider if available, otherwise fall back to default circle layout
    for (uint16_t i = 0; i < _num_leds; i++) {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        
        // If coordinate provider is available, use it
        if (_coordinate_provider) {
            _coordinate_provider(i, x, y, z);
            
            // Scale positions to fit in scene with equal scaling for all axes
            x *= POSITION_SCALE;
            y *= POSITION_SCALE;
            z *= POSITION_SCALE * Z_CORRECTION;
        } else {
            float angle = 2.0f * M_PI * static_cast<float>(i) / static_cast<float>(_num_leds);
            x = cos(angle);
            y = sin(angle);
            z = 0.0f;
        }
        
        // Convert LED color from 0-255 to 0-1 range and apply brightness
        float brightness = static_cast<float>(_brightness) / 255.0f;
        vertices[i].x = x;
        vertices[i].y = y;
        vertices[i].z = z;
        vertices[i].r = static_cast<float>(_leds[i].r) / 255.0f * brightness;
        vertices[i].g = static_cast<float>(_leds[i].g) / 255.0f * brightness;
        vertices[i].b = static_cast<float>(_leds[i].b) / 255.0f * brightness;
    }
    
    // Create or update the vertex buffer
    if (_vertex_buffer == 0) {
        _vertex_buffer = _renderer.createBuffer();
    }
    
    // Update the vertex buffer
    _renderer.bindArrayBuffer(_vertex_buffer, vertices.data(), vertices.size() * sizeof(Vertex), true);
}

void WebPlatform::renderFrame() {
    // Begin render pass
    _renderer.beginRenderPass();
    
    // Calculate projection and view matrices
    float projection[16];
    float view[16];
    float modelRotation[16];
    
    // Update camera distance from platform setting
    _camera.setDistance(_camera_distance);
    
    // Setup perspective projection
    float aspect = static_cast<float>(_canvas_width) / static_cast<float>(_canvas_height);
    float fovRadians = CAMERA_FOV_DEGREES * (M_PI / 180.0f);
    
    // Calculate perspective projection matrix
    Math::perspective(projection, fovRadians, aspect, CAMERA_NEAR_PLANE, CAMERA_FAR_PLANE);
    
    // Get camera view matrix and model rotation matrix separately
    _camera.calculateViewMatrix(view);
    _camera.getModelRotationMatrix(modelRotation);
    
    // Render mesh if enabled
    if (_show_mesh) {
        glUseProgram(_mesh_shader_program);
        
        // Set uniforms
        GLint projMatrixLoc = glGetUniformLocation(_mesh_shader_program, "projection");
        GLint viewMatrixLoc = glGetUniformLocation(_mesh_shader_program, "view");
        GLint modelMatrixLoc = glGetUniformLocation(_mesh_shader_program, "model");
        GLint opacityLoc = glGetUniformLocation(_mesh_shader_program, "mesh_opacity");
        
        glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, projection);
        glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, view);
        glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, modelRotation);
        glUniform1f(opacityLoc, _mesh_opacity);
        
        // Draw mesh using vertex attributes setup by the renderer
        const auto& vertices = _mesh_generator.getVertices();
        const auto& indices = _mesh_generator.getIndices();
        
        if (!vertices.empty() && !indices.empty()) {
            // Create temporary VAO and VBO for the mesh
            uint32_t mesh_vao = _renderer.createVertexArray();
            uint32_t mesh_vbo = _renderer.createBuffer();
            uint32_t mesh_ebo = _renderer.createBuffer();
            
            // Bind and set mesh data
            _renderer.bindArrayBuffer(mesh_vbo, vertices.data(), vertices.size() * sizeof(float));
            _renderer.configureVertexAttributes(mesh_vao, mesh_vbo, true);
            
            // Use the renderer's bindElementBuffer method
            _renderer.bindElementBuffer(mesh_ebo, indices.data(), indices.size() * sizeof(uint16_t));
            
            // Draw mesh
            glBindVertexArray(mesh_vao);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), 
                          GL_UNSIGNED_SHORT, 0);
            
            // Clean up
            glDeleteBuffers(1, &mesh_ebo);
            glDeleteBuffers(1, &mesh_vbo);
            glDeleteVertexArrays(1, &mesh_vao);
        }
    }
    
    // Enable depth testing for proper depth sorting
    glEnable(GL_DEPTH_TEST);
    // Enable additive blending for LED points to create a glowing effect
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glUseProgram(_shader_program);
    
    // Set uniforms for LED rendering
    GLint projLoc = glGetUniformLocation(_shader_program, "projection");
    GLint viewLoc = glGetUniformLocation(_shader_program, "view");
    GLint modelLoc = glGetUniformLocation(_shader_program, "model");
    GLint sizeLoc = glGetUniformLocation(_shader_program, "led_size");
    GLint distanceLoc = glGetUniformLocation(_shader_program, "camera_distance");
    GLint heightLoc = glGetUniformLocation(_shader_program, "canvas_height");
    
    // Add brightness uniform
    GLint brightnessLoc = glGetUniformLocation(_shader_program, "brightness");
    
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelRotation);
    glUniform1f(sizeLoc, _led_size * PHYSICAL_LED_DIAMETER);
    glUniform1f(distanceLoc, _camera_distance);
    glUniform1f(heightLoc, static_cast<float>(_canvas_height));
    
    // Only set brightness if the location is valid
    if (brightnessLoc >= 0) {
        glUniform1f(brightnessLoc, static_cast<float>(_brightness) / 255.0f);
    }
    
    // Configure vertex attributes specifically for LEDs
    uint32_t led_vao = _renderer.createVertexArray();
    _renderer.configureVertexAttributes(led_vao, _vertex_buffer);
    
    // Draw points for each LED
    glBindVertexArray(led_vao);
    glDrawArrays(GL_POINTS, 0, _num_leds);
    
    // Restore default blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Clean up
    glDeleteVertexArrays(1, &led_vao);
    
    // End render pass
    _renderer.endRenderPass();
    
    // Apply post-processing
    _renderer.applyPostProcessing(_glow_shader_program, _atmosphere_intensity);
}

void WebPlatform::updateAutoRotation() {
    if (_auto_rotation) {
        // Get the current time and calculate delta time properly
        double current_time = get_current_time();
        float delta_time = static_cast<float>(current_time - _last_auto_rotation_time);
        
        // Update the timestamp for next frame
        _last_auto_rotation_time = current_time;
        
        // Use camera's built-in auto-rotation method
        _camera.updateAutoRotation(delta_time);
    }
}

// Rotation and view control
void WebPlatform::updateRotation(float deltaX, float deltaY) {
    // Convert mouse movement to rotation angles
    // Scale the movement for smoother control
    float rotationScale = ROTATION_SCALE;
    _camera.updateModelRotation(deltaX * rotationScale, deltaY * rotationScale);
}

void WebPlatform::resetRotation() {
    // Reset model rotation to 0
    _camera.resetModelRotation();
}

void WebPlatform::setAutoRotation(bool enabled, float speed) {
    _auto_rotation = enabled;
    
    // Update the camera's auto-rotation state
    _camera.setAutoRotation(enabled);
    
    // Set the appropriate speed based on the input parameter
    // speed == 1 means slow, speed == 3 means fast
    if (enabled) {
        float rotation_speed = (speed == 1.0f) ? Camera::SLOW_ROTATION_SPEED : Camera::FAST_ROTATION_SPEED;
        _camera.setAutoRotationSpeed(rotation_speed);
    }
    
    // Reset the auto rotation timer to now
    _last_auto_rotation_time = get_current_time();
}

void WebPlatform::setPresetView(int preset_index) {
    // Set camera to a preset view
    switch (preset_index) {
        case 0: // Side view
            _camera.setPresetView(Camera::ViewPreset::SIDE);
            break;
        case 1: // Top view
            _camera.setPresetView(Camera::ViewPreset::TOP);
            break;
        case 2: // Angled view
            _camera.setPresetView(Camera::ViewPreset::ANGLE);
            break;
        default:
            _camera.setPresetView(Camera::ViewPreset::SIDE);
            break;
    }
}

void WebPlatform::setZoomLevel(int zoom_level) {
    // Set camera zoom level
    switch (static_cast<ZoomLevel>(zoom_level)) {
        case ZoomLevel::CLOSE:
            _camera_distance = CAMERA_CLOSE_DISTANCE;
            break;
        case ZoomLevel::NORMAL:
            _camera_distance = CAMERA_NORMAL_DISTANCE;
            break;
        case ZoomLevel::FAR:
            _camera_distance = CAMERA_FAR_DISTANCE;
            break;
        default:
            _camera_distance = CAMERA_NORMAL_DISTANCE;
            break;
    }
    
    // Update the camera
    _camera.setDistance(_camera_distance);
}

// JavaScript interface methods
void WebPlatform::onCanvasResize(int width, int height) {
    _canvas_width = width;
    _canvas_height = height;
    
    // Update renderer viewport
    _renderer.updateViewport(width, height);
}

void WebPlatform::onMouseDown(int x, int y) {
    _mouse_down = true;
    _last_mouse_x = x;
    _last_mouse_y = y;
    
    // Disable auto-rotation when user interacts with mouse
    if (_auto_rotation) {
        _auto_rotation = false;
        _camera.setAutoRotation(false);
    }
}

void WebPlatform::onMouseMove(int x, int y, bool shift_key) {
    if (_mouse_down) {
        float deltaX = static_cast<float>(x - _last_mouse_x);
        float deltaY = static_cast<float>(y - _last_mouse_y);
        
        // Update rotation based on mouse movement
        updateRotation(deltaX, deltaY);
        
        _last_mouse_x = x;
        _last_mouse_y = y;
    }
}

void WebPlatform::onMouseUp() {
    _mouse_down = false;
}

void WebPlatform::onMouseWheel(float delta) {
    // Adjust camera distance based on wheel delta
    _camera_distance = std::max(CAMERA_CLOSE_DISTANCE, 
                               std::min(CAMERA_FAR_DISTANCE, 
                                      _camera_distance - delta * 0.1f));
    _camera.setDistance(_camera_distance);
}
#endif

} // namespace PixelTheater 