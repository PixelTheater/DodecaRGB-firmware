#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)

#include "PixelTheater/platform/web_platform.h"
#include "PixelTheater/platform/webgl/renderer.h"
#include "PixelTheater/platform/webgl/mesh.h"
#include "PixelTheater/platform/webgl/camera.h"
#include "PixelTheater/platform/webgl/shaders.h"
#include "PixelTheater/platform/webgl/math.h"
#include "PixelTheater/platform/webgl/util.h"
#include "PixelTheater/core/log.h"
#include <emscripten.h>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <chrono>   // For millis() stub
#include <cstdlib>  // For rand() stub
#include <random>   // For better random stubs if needed
#include <limits>   // For numeric_limits

#include <GLES3/gl3.h>
#include <emscripten/html5.h>

namespace PixelTheater {

// External C functions for JavaScript interop
extern "C" {
    EMSCRIPTEN_KEEPALIVE int get_canvas_width() {
        return 800; // Default value, will be overridden by JavaScript
    }
    
    EMSCRIPTEN_KEEPALIVE int get_canvas_height() {
        return 600; // Default value, will be overridden by JavaScript
    }
    
    EMSCRIPTEN_KEEPALIVE double get_current_time() {
        double now_ms = emscripten_get_now();
        return now_ms / 1000.0; // Convert to seconds
    }
    
    EMSCRIPTEN_KEEPALIVE void update_ui_brightness(float brightness) {
        // Will be overridden by JavaScript
    }
}

// Define logging macros for stubs if not in web environment
#if !defined(PLATFORM_WEB) && !defined(EMSCRIPTEN)
#define LOG_STUB_PREFIX "[WebPlatStub] "
#endif

WebPlatform::WebPlatform()
    : _leds(nullptr),
      _num_leds(0),
      _brightness(DEFAULT_BRIGHTNESS),
      _led_size(DEFAULT_LED_SIZE),
      _atmosphere_intensity(DEFAULT_ATMOSPHERE_INTENSITY),
      _led_spacing(DEFAULT_LED_SPACING)
{
    // Seed random number generator for stubs
    #if !defined(PLATFORM_WEB) && !defined(EMSCRIPTEN)
    std::srand(std::time(nullptr)); 
    #endif
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
    _led_positions.clear();
    _led_positions.reserve(model.leds.positions.size());
    for (const auto& web_vertex : model.leds.positions) {
        _led_positions.push_back(Point(web_vertex.x, web_vertex.y, web_vertex.z));
    }
    
    // Set initial camera position
    if (_camera) {
        _camera->setDistance(CAMERA_NORMAL_DISTANCE);
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
    // logInfo("WebPlatform::show() entered."); // Log entry - REMOVED (too noisy)

    // Initialize WebGL if not already done
    if (!_renderer || !_mesh_generator) {
        logWarning("WebPlatform::show() returning early: Renderer (%p) or MeshGenerator (%p) not initialized.", 
                   _renderer.get(), _mesh_generator.get());
        return; 
    }
    
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
}

void WebPlatform::setBrightness(uint8_t brightness) {
    _brightness = brightness;
    update_ui_brightness(static_cast<float>(brightness) / 255.0f);

    // Initialize timing variables - REMOVED FPS variables
    _last_auto_rotation_time = WebGLUtil::getCurrentTime(); // Keep for rotation
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
    logInfo("Atmosphere Intensity set to: %.2f (Clamped value)", _atmosphere_intensity);
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
    _camera->resetRotation();
}

void WebPlatform::setAutoRotation(bool enabled, float speed) {
    _auto_rotation = enabled;
    _auto_rotation_speed = speed;
    if (_camera) { // Check if camera exists
        _camera->setAutoRotationSpeed(speed);
    }
    if (enabled) {
        // Reset last time to start rotation smoothly
        _last_auto_rotation_time = get_current_time(); 
    }
}

void WebPlatform::setZoomLevel(int zoom_level) {
    // Convert zoom level (e.g., 0, 1, 2) to camera distance
    float distance = CAMERA_FAR_DISTANCE - zoom_level * (CAMERA_FAR_DISTANCE - CAMERA_CLOSE_DISTANCE) / 2.0f;
    distance = std::clamp(distance, CAMERA_CLOSE_DISTANCE, CAMERA_FAR_DISTANCE);
    if (_camera) { // Check if camera exists
         _camera->setDistance(distance);
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
    _is_dragging = true;
    _last_mouse_x = x;
    _last_mouse_y = y;
    
    // Disable auto-rotation when user interacts with mouse
    if (_auto_rotation) {
        _auto_rotation = false;
        _camera->setAutoRotation(false);
    }
}

void WebPlatform::onMouseMove(int x, int y, bool shift_key) {
    if (_is_dragging) {
        float deltaX = static_cast<float>(x - _last_mouse_x);
        float deltaY = static_cast<float>(y - _last_mouse_y);
        
        // Update camera rotation
        updateRotation(deltaX, deltaY);
        
        _last_mouse_x = x;
        _last_mouse_y = y;
    }
}

void WebPlatform::onMouseUp() {
    _is_dragging = false;
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
    if (!_leds || !_renderer) return;
    
    // Define vertex structure for LED points (Reverted)
    struct Vertex {
        float x, y, z;  // 3D coordinates
        float r, g, b;  // Color
    };
    
    // Create vertex data (Reverted: 1 vertex per LED)
    std::vector<Vertex> vertices(_num_leds);
    
    // Scale factor to fit LEDs in scene (Reverted)
    constexpr float POSITION_SCALE = 0.03f;
    // Anti-clipping scale factor
    constexpr float CLIP_SCALE = 1.025f; // Increased slightly

    // Use cached LED positions from WebModel
    for (uint16_t i = 0; i < _num_leds; i++) {
        // Original LED position from model
        Point pos = _led_positions[i];

        // Apply anti-clipping scale and main position scale during vertex creation
        vertices[i].x = pos.x() * CLIP_SCALE * POSITION_SCALE;
        vertices[i].y = pos.y() * CLIP_SCALE * POSITION_SCALE;
        vertices[i].z = pos.z() * CLIP_SCALE * POSITION_SCALE;
        
        // Convert LED color from 0-255 to 0-1 range and apply brightness (Reverted)
        float brightness = static_cast<float>(_brightness) / 255.0f;
        vertices[i].r = static_cast<float>(_leds[i].r) / 255.0f * brightness;
        vertices[i].g = static_cast<float>(_leds[i].g) / 255.0f * brightness;
        vertices[i].b = static_cast<float>(_leds[i].b) / 255.0f * brightness;
    }
    
    // Create or update the vertex buffer
    if (_led_vbo == 0) {
        _led_vbo = _renderer->createBuffer();
    }
    
    // Update the vertex buffer (Reverted: Use Vertex struct size)
    _renderer->bindArrayBuffer(_led_vbo, vertices.data(), vertices.size() * sizeof(Vertex), true);
}

void WebPlatform::renderMesh(const float* view_matrix, const float* projection_matrix, const float* model_matrix) {
    if (!_mesh_generator || !_renderer) return;
    
    // Get mesh data
    const std::vector<float>& vertices = _mesh_generator->getVertices();
    const std::vector<uint16_t>& indices = _mesh_generator->getIndices();
    const std::vector<float>& edge_vertices = _mesh_generator->getEdgeVertices();
    const std::vector<uint16_t>& edge_indices = _mesh_generator->getEdgeIndices();
    
    // Render mesh faces if enabled
    if (!vertices.empty() && !indices.empty()) {
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
        GLint isWireframeLoc = glGetUniformLocation(_mesh_shader_program, "is_wireframe");
        
        if (projLoc >= 0) glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection_matrix);
        if (viewLoc >= 0) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view_matrix);
        if (modelLoc >= 0) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model_matrix);
        if (opacityLoc >= 0) glUniform1f(opacityLoc, _mesh_opacity);
        if (colorLoc >= 0) glUniform3f(colorLoc, 0.0f, 0.2f, 0.15f); // Darker, less saturated green for faces
        if (lightPosLoc >= 0) glUniform3f(lightPosLoc, 0.0f, 2.0f, 2.0f); // Set a default light position
        if (isWireframeLoc >= 0) glUniform1i(isWireframeLoc, 0); // Set to false (0) for faces
        
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
        
        // Set uniforms for wireframe: Always white and opaque
        GLint projLoc = glGetUniformLocation(_mesh_shader_program, "projection");
        GLint viewLoc = glGetUniformLocation(_mesh_shader_program, "view");
        GLint modelLoc = glGetUniformLocation(_mesh_shader_program, "model");
        GLint opacityLoc = glGetUniformLocation(_mesh_shader_program, "mesh_opacity"); // Still use this uniform name
        GLint colorLoc = glGetUniformLocation(_mesh_shader_program, "mesh_color");
        GLint isWireframeLoc = glGetUniformLocation(_mesh_shader_program, "is_wireframe");
        
        if (projLoc >= 0) glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection_matrix);
        if (viewLoc >= 0) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view_matrix);
        if (modelLoc >= 0) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model_matrix);
        if (opacityLoc >= 0) glUniform1f(opacityLoc, 1.0f); // Always fully opaque
        if (colorLoc >= 0) glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f); // Always white
        if (isWireframeLoc >= 0) glUniform1i(isWireframeLoc, 1); // Set to true (1) for wireframe
        
        // Create and bind VAO and VBO for edges
        GLuint edge_vao, edge_vbo, edge_ebo;
        glGenVertexArrays(1, &edge_vao);
        glGenBuffers(1, &edge_vbo);
        glGenBuffers(1, &edge_ebo);
        
        glBindVertexArray(edge_vao);
        
        // Bind vertex buffer, apply nudge first
        {
            // Define the same scale factor used for LEDs
            constexpr float CLIP_SCALE = 1.025f; 
            std::vector<float> nudged_edge_vertices;
            nudged_edge_vertices.reserve(edge_vertices.size());

            for (size_t i = 0; i < edge_vertices.size(); i += 3) {
                float x = edge_vertices[i];
                float y = edge_vertices[i+1];
                float z = edge_vertices[i+2];
                // Normalize, scale, then multiply (Alternative: just scale)
                // Scaling position vector directly might be simpler/sufficient
                nudged_edge_vertices.push_back(x * CLIP_SCALE);
                nudged_edge_vertices.push_back(y * CLIP_SCALE);
                nudged_edge_vertices.push_back(z * CLIP_SCALE);
            }

            glBindBuffer(GL_ARRAY_BUFFER, edge_vbo);
            // Upload the nudged vertices
            glBufferData(GL_ARRAY_BUFFER, nudged_edge_vertices.size() * sizeof(float), nudged_edge_vertices.data(), GL_STATIC_DRAW);
        }
        
        // Bind element buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edge_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, edge_indices.size() * sizeof(uint16_t), edge_indices.data(), GL_STATIC_DRAW);
        
        // Set vertex attributes for edges (position only, attribute location 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // Only position data (x, y, z)
        glEnableVertexAttribArray(0);
        // Ensure normal attribute (location 1) is disabled for wireframe
        glDisableVertexAttribArray(1); 
        
        // Draw wireframe lines (glLineWidth > 1.0 unreliable in WebGL)
        // glLineWidth(5.0f); // Try increasing line width further
        glDrawElements(GL_LINES, edge_indices.size(), GL_UNSIGNED_SHORT, 0);
        // glLineWidth(1.0f); // Reset line width
        
        // Clean up edge buffers
        glDeleteVertexArrays(1, &edge_vao);
        glDeleteBuffers(1, &edge_vbo);
        glDeleteBuffers(1, &edge_ebo);
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

    glUseProgram(_led_shader_program);
    
    // Set uniforms for LED rendering (Reverted: Removed aspectLoc)
    GLint projLoc = glGetUniformLocation(_led_shader_program, "projection");
    GLint viewLoc = glGetUniformLocation(_led_shader_program, "view");
    GLint modelLoc = glGetUniformLocation(_led_shader_program, "model");
    GLint sizeLoc = glGetUniformLocation(_led_shader_program, "led_size");
    GLint distanceLoc = glGetUniformLocation(_led_shader_program, "camera_distance");
    GLint heightLoc = glGetUniformLocation(_led_shader_program, "canvas_height");
    GLint brightnessLoc = glGetUniformLocation(_led_shader_program, "brightness");
    // REMOVED: GLint aspectLoc = glGetUniformLocation(_led_shader_program, "aspect_ratio");
    
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection_matrix);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view_matrix);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model_matrix);
    glUniform1f(sizeLoc, _led_size * PHYSICAL_LED_DIAMETER);
    glUniform1f(distanceLoc, _camera->getDistance());
    glUniform1f(heightLoc, static_cast<float>(_canvas_height));
    glUniform1f(brightnessLoc, static_cast<float>(_brightness) / 255.0f); 
    // REMOVED: Aspect ratio uniform setting

    // --- Bind VAO/VBO and Set Attributes for Points (Reverted) ---
    // Define vertex structure to match updateVertexBuffer (Reverted)
    struct Vertex {
        float x, y, z;  // Position
        float r, g, b;  // Color
    };

    if (_led_vbo == 0 || _led_vao == 0) {
        logError("renderLEDs: LED VBO or VAO not initialized!");
        return; // Cannot render without buffers
    }

    glBindVertexArray(_led_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _led_vbo);
    
    // Set attributes for Points: position (loc 0), color (loc 1)
    GLsizei stride = sizeof(Vertex); // Stride is the size of the Vertex struct

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(0);
    
    // Color attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, r));
    glEnableVertexAttribArray(1);

    // Disable attribute location 2 (was used for cornerOffset)
    glDisableVertexAttribArray(2);
    // --- END Reverted Attribute Setup --- 
    
    // Draw Points (Reverted)
    glDrawArrays(GL_POINTS, 0, _num_leds);

    // --- Unbind VAO --- 
    glBindVertexArray(0);
    // --- END ADDED ---

    // Restore default blend function for other rendering
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void WebPlatform::initWebGL() {
    // Get initial canvas dimensions
    _canvas_width = WebGLUtil::getCanvasWidth();
    _canvas_height = WebGLUtil::getCanvasHeight();
    
    // Create WebGL renderer
    if (!_renderer) _renderer = std::make_unique<WebGLRenderer>();
    if (!_renderer->initialize(_canvas_width, _canvas_height)) {
        printf("Failed to initialize WebGL renderer\n");
        return;
    }
    
    // Create camera
    if (!_camera) _camera = std::make_unique<Camera>();
    
    // Set camera distance and position for proper vertical centering
    _camera->setDistance(CAMERA_NORMAL_DISTANCE);
    
    // Create mesh generator
    if (!_mesh_generator) _mesh_generator = std::make_unique<MeshGenerator>();
    
    // Create shader programs
    _led_shader_program = _renderer->createShaderProgram(vertex_shader_source, fragment_shader_source);
    _mesh_shader_program = _renderer->createShaderProgram(mesh_vertex_shader_source, mesh_fragment_shader_source);
    _glow_shader_program = _renderer->createShaderProgram(glow_vertex_shader_source, glow_fragment_shader_source);
    _blur_shader_program = _renderer->createShaderProgram(blur_vertex_shader_source, blur_fragment_shader_source);
    _composite_shader_program = _renderer->createShaderProgram(quad_vertex_shader_source, composite_fragment_shader_source);
    
    // Log Shader Program IDs for Debugging
    logInfo("Initialized Shader Programs: LED=%u, Mesh=%u, Glow=%u, Blur=%u, Composite=%u", 
            _led_shader_program, _mesh_shader_program, _glow_shader_program, 
            _blur_shader_program, _composite_shader_program);
    if (_glow_shader_program == 0) logError("Glow Shader Program failed to compile/link!");

    _led_vbo = _renderer->createBuffer();
    
    // --- ADDED: Create VAO for LEDs --- 
    if (_led_vao == 0) { // Check if not already created (though unlikely here)
        glGenVertexArrays(1, &_led_vao);
        if (_led_vao == 0) { // Check if creation failed
            logError("Failed to generate VAO for LEDs!");
        } 
    }
    
    // Initialize timing variables
    _last_auto_rotation_time = WebGLUtil::getCurrentTime();
}

void WebPlatform::cleanupWebGL() {
    // Clean up shader programs
    if (_led_shader_program) {
        glDeleteProgram(_led_shader_program);
        _led_shader_program = 0;
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
    if (_led_vbo) {
        glDeleteBuffers(1, &_led_vbo);
        _led_vbo = 0;
    }
    
    // Clean up VAO
    if (_led_vao) {
        glDeleteVertexArrays(1, &_led_vao);
        _led_vao = 0;
    }
    
    // Clean up WebGL components (using unique_ptr::reset)
    _renderer.reset();
    _mesh_generator.reset();
    _camera.reset();
}

// Add a new function to update scene parameters
EMSCRIPTEN_KEEPALIVE void update_scene_parameter(const char* param_id, float value) {
    // This function is called from JavaScript
    // We can't directly access the WebSimulator from here
    // Instead, we'll log a message for debugging
    Log::warning("update_scene_parameter called with param_id: %s, value: %f", param_id, value);
    
    // The actual parameter update will be handled by the WebSimulator
    // through the Emscripten bindings
}

void WebPlatform::updateSceneParameter(const char* param_id, float value) {
    // This method is part of the WebPlatform class
    // It's called by the WebSimulator
    // For now, just log the call
    Log::warning("WebPlatform::updateSceneParameter called with param_id: %s, value: %f", param_id, value);
}

// ==============================================================
// Stubs for Platform virtual methods (Native Testing)
// ==============================================================

// Timing Utilities
float WebPlatform::deltaTime() {
    #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    double now = emscripten_get_now(); // Milliseconds
    float dt = 0.0f;
    if (_last_delta_time > 0.0) { // Avoid large delta on first frame
        dt = static_cast<float>(now - _last_delta_time) / 1000.0f; // Convert ms to seconds
    }
    _last_delta_time = now;
    // Clamp delta time to avoid large jumps (e.g., when tab is inactive)
    return std::min(dt, 0.1f); // Max delta time of 100ms (10 FPS)
    #else
    return 1.0f / 60.0f; // Stub for native
    #endif
}

uint32_t WebPlatform::millis() {
    #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    // Use emscripten_get_now() which returns time in milliseconds
    return static_cast<uint32_t>(emscripten_get_now());
    #else
    // Stub for native: return milliseconds since epoch or similar
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    #endif
}

// Random Number Utilities
uint8_t WebPlatform::random8() {
    #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    // Use EM_ASM to call Math.random()
    return static_cast<uint8_t>(EM_ASM_INT({ return Math.floor(Math.random() * 256); }));
    #else
    return static_cast<uint8_t>(std::rand() % 256);
    #endif
}

uint16_t WebPlatform::random16() {
    #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    // Combine two Math.random() calls for better distribution (though still not perfect)
    uint16_t r1 = EM_ASM_INT({ return Math.floor(Math.random() * 256); });
    uint16_t r2 = EM_ASM_INT({ return Math.floor(Math.random() * 256); });
    return (r1 << 8) | r2;
    #else
    return static_cast<uint16_t>(std::rand());
    #endif
}

// Default to RAND_MAX if max is 0
uint32_t WebPlatform::random(uint32_t max) {
    #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    if (max == 0) return 0;
    // Use EM_ASM with modulo. Note: Math.random() * max gives [0, max)
    return EM_ASM_INT({ return Math.floor(Math.random() * $0); }, max);
    #else
    if (max == 0) return 0;
    return std::rand() % max;
    #endif
}

uint32_t WebPlatform::random(uint32_t min, uint32_t max) {
    #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    if (min >= max) return min;
    uint32_t range = max - min;
    // Use EM_ASM: Math.random() * range + min gives [min, max)
    return EM_ASM_INT({ return Math.floor(Math.random() * $0 + $1); }, range, min);
    #else
    if (min >= max) return min;
    uint32_t range = max - min;
    return (std::rand() % range) + min;
    #endif
}

float WebPlatform::randomFloat() {
    #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    return EM_ASM_DOUBLE({ return Math.random(); });
    #else
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    #endif
}

float WebPlatform::randomFloat(float max) {
    #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    return EM_ASM_DOUBLE({ return Math.random() * $0; }, max);
    #else
    return (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * max;
    #endif
}

float WebPlatform::randomFloat(float min, float max) {
    #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
     if (min >= max) return min;
    float range = max - min;
    return EM_ASM_DOUBLE({ return Math.random() * $0 + $1; }, range, min);
    #else
    if (min >= max) return min;
    float range = max - min;
    return (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * range + min;
    #endif
}

// Logging Utilities
void WebPlatform::logInfo(const char* format, ...) {
    va_list args;
    va_start(args, format);
    #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    EM_ASM({
        console.log("[INFO] " + UTF8ToString($0));
    }, buffer);
    #else
    // Native stub - print to stdout
    printf(LOG_STUB_PREFIX "INFO: ");
    vprintf(format, args);
    printf("\n");
    #endif
    va_end(args);
}

void WebPlatform::logWarning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    EM_ASM({
        console.warn("[WARN] " + UTF8ToString($0));
    }, buffer);
    #else
    printf(LOG_STUB_PREFIX "WARN: ");
    vprintf(format, args);
    printf("\n");
    #endif
    va_end(args);
}

void WebPlatform::logError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    #if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    EM_ASM({
        console.error("[ERR ] " + UTF8ToString($0));
    }, buffer);
    #else
    fprintf(stderr, LOG_STUB_PREFIX "ERROR: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    #endif
    va_end(args);
}

} // namespace PixelTheater

#endif // defined(PLATFORM_WEB) || defined(EMSCRIPTEN) 