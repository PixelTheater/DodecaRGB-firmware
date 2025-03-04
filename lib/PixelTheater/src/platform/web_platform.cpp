#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)

#include "PixelTheater/platform/web_platform.h"
#include "PixelTheater/platform/webgl/renderer.h"
#include "PixelTheater/platform/webgl/mesh.h"
#include "PixelTheater/platform/webgl/camera.h"
#include "PixelTheater/platform/webgl/shaders.h"
#include "PixelTheater/platform/webgl/math.h"
#include "PixelTheater/platform/webgl/util.h"
#include <emscripten.h>
#include <cmath>
#include <cstdio>
#include <algorithm>

#include <GLES3/gl3.h>
#include <emscripten/html5.h>

// External C functions for JavaScript interop
extern "C" {
    EMSCRIPTEN_KEEPALIVE int get_canvas_width() {
        return 800; // Default value, will be overridden by JavaScript
    }
    
    EMSCRIPTEN_KEEPALIVE int get_canvas_height() {
        return 600; // Default value, will be overridden by JavaScript
    }
    
    EMSCRIPTEN_KEEPALIVE double get_current_time() {
        return emscripten_get_now() / 1000.0; // Convert to seconds
    }
    
    EMSCRIPTEN_KEEPALIVE void update_ui_fps(int fps) {
        // Will be overridden by JavaScript
    }
    
    EMSCRIPTEN_KEEPALIVE void update_ui_brightness(float brightness) {
        // Will be overridden by JavaScript
    }
}

namespace PixelTheater {
namespace WebGL {

WebPlatform::WebPlatform()
    : _leds(nullptr),
      _num_leds(0),
      _brightness(DEFAULT_BRIGHTNESS),
      _led_size(DEFAULT_LED_SIZE),
      _atmosphere_intensity(DEFAULT_ATMOSPHERE_INTENSITY),
      _led_spacing(DEFAULT_LED_SPACING)
{
    // WebGL initialization will happen when a model is loaded
}

WebPlatform::~WebPlatform() {
    delete[] _leds;
    cleanupWebGL();
}

void WebPlatform::initializeFromWebModel(const WebModel& model) {
    // Clean up any existing resources
    delete[] _leds;
    cleanupWebGL();
    
    // Store model metadata
    _num_leds = model.metadata.num_leds;
    
    // Create LED array
    _leds = new CRGB[_num_leds];
    clear(); // Initialize all LEDs to off
    
    // Initialize WebGL components
    initWebGL();
    
    // Generate mesh from model geometry
    if (_mesh_generator) {
        _mesh_generator->generateDodecahedronMesh(model.geometry.faces);
    }
    
    // Store LED positions for rendering
    // We'll use these in updateVertexBuffer() instead of calling coordinate provider
    _led_positions = model.leds.positions;
    
    // Set initial camera position
    if (_camera) {
        _camera->setDistance(CAMERA_NORMAL_DISTANCE);
        _camera->setPresetView(Camera::ViewPreset::ANGLE);
    }
    
    printf("Initialized WebPlatform with %s v%s (%d LEDs)\n", 
           model.metadata.name.c_str(),
           model.metadata.version.c_str(),
           _num_leds);
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
    // Initialize WebGL if not already done
    if (!_renderer || !_mesh_generator) return;
    
    // Update auto-rotation if enabled
    if (_auto_rotation) {
        double current_time = get_current_time();
        float delta_time = static_cast<float>(current_time - _last_auto_rotation_time);
        _camera->updateAutoRotation(delta_time);
        _last_auto_rotation_time = current_time;
    }
    
    // Begin render pass
    _renderer->beginRenderPass();
    
    // Calculate view and projection matrices
    float view_matrix[16];
    float projection_matrix[16];
    float model_matrix[16];
    
    _camera->calculateViewMatrix(view_matrix);
    _camera->getModelRotationMatrix(model_matrix);
    
    // Calculate projection matrix
    float aspect = static_cast<float>(_canvas_width) / static_cast<float>(_canvas_height);
    float fov_radians = CAMERA_FOV_DEGREES * (M_PI / 180.0f);
    Math::perspective(projection_matrix, fov_radians, aspect, CAMERA_NEAR_PLANE, CAMERA_FAR_PLANE);
    
    // Update vertex buffer with current LED colors
    updateVertexBuffer();
    
    // Render the mesh and wireframe independently
    renderMesh(view_matrix, projection_matrix, model_matrix);
    
    // Render LED points
    renderLEDs(view_matrix, projection_matrix, model_matrix);
    
    // End render pass
    _renderer->applyPostProcessing(_glow_shader_program, _atmosphere_intensity);
    
    // Update FPS counter
    double current_time = get_current_time();
    _frame_count++;
    
    if (current_time - _last_frame_time >= 1.0) {
        int fps = static_cast<int>(_frame_count / (current_time - _last_frame_time));
        update_ui_fps(fps);
        _frame_count = 0;
        _last_frame_time = current_time;
    }
}

void WebPlatform::setBrightness(uint8_t brightness) {
    _brightness = brightness;
    update_ui_brightness(static_cast<float>(brightness) / 255.0f);
}

uint8_t WebPlatform::getBrightness() const {
    return _brightness;
}

void WebPlatform::clear() {
    if (_leds) {
        for (uint16_t i = 0; i < _num_leds; i++) {
            _leds[i] = CRGB(0, 0, 0);
        }
    }
}

// Performance settings
void WebPlatform::setMaxRefreshRate(uint8_t fps) {
    _max_refresh_rate = fps;
}

void WebPlatform::setDither(uint8_t dither) {
    _dither = dither;
}

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
    _mesh_opacity = opacity;
}

float WebPlatform::getMeshOpacity() const {
    return _mesh_opacity;
}

void WebPlatform::setShowWireframe(bool show) {
    _show_wireframe = show;
}

bool WebPlatform::getShowWireframe() const {
    return _show_wireframe;
}

// Rotation and view control
void WebPlatform::updateRotation(float deltaX, float deltaY) {
    // Convert mouse movement to rotation angles
    // Scale the movement for smoother control
    float rotationScale = ROTATION_SCALE;
    _camera->updateModelRotation(deltaX * rotationScale, deltaY * rotationScale);
}

void WebPlatform::resetRotation() {
    // Reset model rotation to 0
    _camera->resetModelRotation();
}

void WebPlatform::setAutoRotation(bool enabled, float speed) {
    _auto_rotation = enabled;
    
    // Update the camera's auto-rotation state
    _camera->setAutoRotation(enabled);
    
    // Set the appropriate speed based on the input parameter
    // speed == 1 means slow, speed == 3 means fast
    if (enabled) {
        float rotation_speed = (speed == 1.0f) ? Camera::SLOW_ROTATION_SPEED : Camera::FAST_ROTATION_SPEED;
        _camera->setAutoRotationSpeed(rotation_speed);
    }
    
    // Reset the auto rotation timer to now
    _last_auto_rotation_time = get_current_time();
}

void WebPlatform::setPresetView(int preset_index) {
    // Set camera to a preset view
    switch (preset_index) {
        case 0: // Side view
            _camera->setPresetView(Camera::ViewPreset::SIDE);
            break;
        case 1: // Top view
            _camera->setPresetView(Camera::ViewPreset::TOP);
            break;
        case 2: // Angled view
            _camera->setPresetView(Camera::ViewPreset::ANGLE);
            break;
        default:
            _camera->setPresetView(Camera::ViewPreset::SIDE);
            break;
    }
}

void WebPlatform::setZoomLevel(int zoom_level) {
    // Set camera zoom level
    switch (static_cast<ZoomLevel>(zoom_level)) {
        case ZoomLevel::CLOSE:
            _camera->setDistance(CAMERA_CLOSE_DISTANCE);
            break;
        case ZoomLevel::NORMAL:
            _camera->setDistance(CAMERA_NORMAL_DISTANCE);
            break;
        case ZoomLevel::FAR:
            _camera->setDistance(CAMERA_FAR_DISTANCE);
            break;
        default:
            _camera->setDistance(CAMERA_NORMAL_DISTANCE);
            break;
    }
}

// JavaScript interface methods
void WebPlatform::onCanvasResize(int width, int height) {
    _canvas_width = width;
    _canvas_height = height;
    
    // Update renderer viewport
    _renderer->updateViewport(width, height);
}

void WebPlatform::onMouseDown(int x, int y) {
    _mouse_down = true;
    _last_mouse_x = x;
    _last_mouse_y = y;
    
    // Disable auto-rotation when user interacts with mouse
    if (_auto_rotation) {
        _auto_rotation = false;
        _camera->setAutoRotation(false);
    }
}

void WebPlatform::onMouseMove(int x, int y, bool shift_key) {
    if (_mouse_down) {
        float deltaX = static_cast<float>(x - _last_mouse_x);
        float deltaY = static_cast<float>(y - _last_mouse_y);
        
        // Update camera rotation
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
    float current_distance = _camera->getDistance();
    float new_distance = current_distance * (1.0f + delta * 0.1f);
    
    // Clamp distance between close and far limits
    new_distance = std::max(CAMERA_CLOSE_DISTANCE, std::min(CAMERA_FAR_DISTANCE, new_distance));
    _camera->setDistance(new_distance);
}

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
    
    // Use cached LED positions from WebModel
    for (uint16_t i = 0; i < _num_leds; i++) {
        // Scale positions to fit in scene with equal scaling for all axes
        vertices[i].x = _led_positions[i].x * POSITION_SCALE;
        vertices[i].y = _led_positions[i].y * POSITION_SCALE;
        vertices[i].z = _led_positions[i].z * POSITION_SCALE * Z_CORRECTION;
        
        // Convert LED color from 0-255 to 0-1 range and apply brightness
        float brightness = static_cast<float>(_brightness) / 255.0f;
        vertices[i].r = static_cast<float>(_leds[i].r) / 255.0f * brightness;
        vertices[i].g = static_cast<float>(_leds[i].g) / 255.0f * brightness;
        vertices[i].b = static_cast<float>(_leds[i].b) / 255.0f * brightness;
    }
    
    // Create or update the vertex buffer
    if (_vertex_buffer == 0) {
        _vertex_buffer = _renderer->createBuffer();
    }
    
    // Update the vertex buffer
    _renderer->bindArrayBuffer(_vertex_buffer, vertices.data(), vertices.size() * sizeof(Vertex), true);
}

void WebPlatform::renderMesh(const float* view_matrix, const float* projection_matrix, const float* model_matrix) {
    if (!_mesh_generator || !_renderer) return;
    
    // Get mesh data
    const std::vector<float>& vertices = _mesh_generator->getVertices();
    const std::vector<uint16_t>& indices = _mesh_generator->getIndices();
    const std::vector<float>& edge_vertices = _mesh_generator->getEdgeVertices();
    const std::vector<uint16_t>& edge_indices = _mesh_generator->getEdgeIndices();
    
    // Render mesh faces if enabled
    if (_show_mesh && !vertices.empty() && !indices.empty()) {
        // Enable depth testing and blending for mesh faces
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Use mesh shader program
        glUseProgram(_mesh_shader_program);
        
        // Set uniforms
        GLint projLoc = glGetUniformLocation(_mesh_shader_program, "projection");
        GLint viewLoc = glGetUniformLocation(_mesh_shader_program, "view");
        GLint modelLoc = glGetUniformLocation(_mesh_shader_program, "model");
        GLint opacityLoc = glGetUniformLocation(_mesh_shader_program, "mesh_opacity");
        GLint colorLoc = glGetUniformLocation(_mesh_shader_program, "mesh_color");
        GLint lightPosLoc = glGetUniformLocation(_mesh_shader_program, "light_position");
        
        if (projLoc >= 0) glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection_matrix);
        if (viewLoc >= 0) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view_matrix);
        if (modelLoc >= 0) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model_matrix);
        if (opacityLoc >= 0) glUniform1f(opacityLoc, _mesh_opacity);
        if (colorLoc >= 0) glUniform3f(colorLoc, 0.8f, 0.8f, 0.8f); // Light gray
        if (lightPosLoc >= 0) glUniform3f(lightPosLoc, 0.0f, 2.0f, 2.0f);
        
        // Create and bind VAO and VBO for mesh faces
        GLuint vao, vbo, ebo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        
        glBindVertexArray(vao);
        
        // Bind vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        // Bind element buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);
        
        // Set vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        // Draw mesh faces
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);
        
        // Clean up mesh face buffers
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }
    
    // Render wireframe edges if enabled
    if (_show_wireframe && !edge_vertices.empty() && !edge_indices.empty()) {
        // Enable depth testing and blending for edges
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Use mesh shader program
        glUseProgram(_mesh_shader_program);
        
        // Set uniforms
        GLint projLoc = glGetUniformLocation(_mesh_shader_program, "projection");
        GLint viewLoc = glGetUniformLocation(_mesh_shader_program, "view");
        GLint modelLoc = glGetUniformLocation(_mesh_shader_program, "model");
        GLint opacityLoc = glGetUniformLocation(_mesh_shader_program, "mesh_opacity");
        GLint colorLoc = glGetUniformLocation(_mesh_shader_program, "mesh_color");
        
        if (projLoc >= 0) glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection_matrix);
        if (viewLoc >= 0) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view_matrix);
        if (modelLoc >= 0) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model_matrix);
        if (opacityLoc >= 0) glUniform1f(opacityLoc, 0.9f); // More opaque edges
        if (colorLoc >= 0) glUniform3f(colorLoc, 0.95f, 0.95f, 0.95f); // Almost white
        
        // Create and bind VAO and VBO for edges
        GLuint edge_vao, edge_vbo, edge_ebo;
        glGenVertexArrays(1, &edge_vao);
        glGenBuffers(1, &edge_vbo);
        glGenBuffers(1, &edge_ebo);
        
        glBindVertexArray(edge_vao);
        
        // Bind vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, edge_vbo);
        glBufferData(GL_ARRAY_BUFFER, edge_vertices.size() * sizeof(float), edge_vertices.data(), GL_STATIC_DRAW);
        
        // Bind element buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edge_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, edge_indices.size() * sizeof(uint16_t), edge_indices.data(), GL_STATIC_DRAW);
        
        // Set vertex attributes for edges (position only)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glDisableVertexAttribArray(1); // Disable normal attribute for edges
        
        // Set line width
        glLineWidth(2.0f);
        
        // Draw edges
        glDrawElements(GL_LINES, edge_indices.size(), GL_UNSIGNED_SHORT, 0);
        
        // Clean up edge buffers
        glDeleteVertexArrays(1, &edge_vao);
        glDeleteBuffers(1, &edge_vbo);
        glDeleteBuffers(1, &edge_ebo);
        
        // Restore default line width
        glLineWidth(1.0f);
    }
    
    // Restore default blend function
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void WebPlatform::renderLEDs(const float* view_matrix, const float* projection_matrix, const float* model_matrix) {
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
    
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection_matrix);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view_matrix);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model_matrix);
    glUniform1f(sizeLoc, _led_size * PHYSICAL_LED_DIAMETER);
    glUniform1f(distanceLoc, _camera->getDistance());
    glUniform1f(heightLoc, static_cast<float>(_canvas_height));
    
    // Only set brightness if the location is valid
    if (brightnessLoc >= 0) {
        glUniform1f(brightnessLoc, static_cast<float>(_brightness) / 255.0f);
    }
    
    // Configure vertex attributes specifically for LEDs
    uint32_t led_vao = _renderer->createVertexArray();
    _renderer->configureVertexAttributes(led_vao, _vertex_buffer);
    
    // Draw points for each LED
    glBindVertexArray(led_vao);
    glDrawArrays(GL_POINTS, 0, _num_leds);
    
    // Restore default blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Clean up
    glDeleteVertexArrays(1, &led_vao);
}

void WebPlatform::initWebGL() {
    // Get initial canvas dimensions
    _canvas_width = WebGLUtil::getCanvasWidth();
    _canvas_height = WebGLUtil::getCanvasHeight();
    
    // Create WebGL renderer
    _renderer = std::make_unique<WebGLRenderer>();
    if (!_renderer->initialize(_canvas_width, _canvas_height)) {
        printf("Failed to initialize WebGL renderer\n");
        return;
    }
    
    // Create camera
    _camera = std::make_unique<Camera>();
    _camera->setDistance(CAMERA_NORMAL_DISTANCE);
    
    // Create mesh generator
    _mesh_generator = std::make_unique<MeshGenerator>();
    
    // Create shader programs
    _shader_program = _renderer->createShaderProgram(vertex_shader_source, fragment_shader_source);
    _mesh_shader_program = _renderer->createShaderProgram(mesh_vertex_shader_source, mesh_fragment_shader_source);
    _glow_shader_program = _renderer->createShaderProgram(glow_vertex_shader_source, glow_fragment_shader_source);
    _blur_shader_program = _renderer->createShaderProgram(blur_vertex_shader_source, blur_fragment_shader_source);
    _composite_shader_program = _renderer->createShaderProgram(quad_vertex_shader_source, composite_fragment_shader_source);
    
    // Create vertex buffer for LED positions and colors
    _vertex_buffer = _renderer->createBuffer();
    
    // Initialize timing variables
    _last_frame_time = WebGLUtil::getCurrentTime();
    _last_auto_rotation_time = _last_frame_time;
}

void WebPlatform::cleanupWebGL() {
    // Clean up shader programs
    if (_shader_program) {
        glDeleteProgram(_shader_program);
        _shader_program = 0;
    }
    if (_mesh_shader_program) {
        glDeleteProgram(_mesh_shader_program);
        _mesh_shader_program = 0;
    }
    if (_glow_shader_program) {
        glDeleteProgram(_glow_shader_program);
        _glow_shader_program = 0;
    }
    if (_blur_shader_program) {
        glDeleteProgram(_blur_shader_program);
        _blur_shader_program = 0;
    }
    if (_composite_shader_program) {
        glDeleteProgram(_composite_shader_program);
        _composite_shader_program = 0;
    }
    
    // Clean up vertex buffer
    if (_vertex_buffer) {
        glDeleteBuffers(1, &_vertex_buffer);
        _vertex_buffer = 0;
    }
    
    // Clean up WebGL components
    _renderer.reset();
    _mesh_generator.reset();
    _camera.reset();
}

} // namespace WebGL
} // namespace PixelTheater 

#endif // defined(PLATFORM_WEB) || defined(EMSCRIPTEN) 