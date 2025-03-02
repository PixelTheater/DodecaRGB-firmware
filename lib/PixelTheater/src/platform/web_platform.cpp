#include "PixelTheater/platform/web_platform.h"
#include "PixelTheater/core/color.h"

#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
// =========================================================================
// FULL WEB PLATFORM IMPLEMENTATION (Only compiled in web builds)
// =========================================================================

#include <emscripten.h>
#include <emscripten/html5.h>
#include <iostream>
#include <cmath>

// External declaration of debug mode flag
extern bool g_debug_mode;

// Forward declaration for the model
namespace PixelTheater {
    template<typename ModelDef>
    class Model;
}

// External model reference - defined in web_simulator.cpp
extern void* g_model_ptr;

namespace PixelTheater {

// Vertex shader source with 3D projection
const char* vertex_shader_source = R"(#version 300 es
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
out vec3 fragColor;
out float depth;
uniform float pointSize;  // Uniform for point size
uniform mat4 projection;  // Projection matrix
uniform mat4 view;        // View matrix
uniform float time;       // Time for animation

void main() {
    // Apply transformations
    vec4 viewPos = view * vec4(position, 1.0);
    gl_Position = projection * viewPos;
    
    // Calculate depth for fragment shader (to fade distant points)
    depth = -viewPos.z;
    
    // Adjust point size based on z-coordinate (perspective)
    // Use a gentler scaling to maintain visibility at different zoom levels
    float sizeScale = 1.0 - gl_Position.z * 0.1; // Reduced from 0.1 for more consistent size
    gl_PointSize = pointSize * max(sizeScale, 0.15); // Increased minimum scale from 0.1
    
    fragColor = color;
}
)";

// Fragment shader source
const char* fragment_shader_source = R"(#version 300 es
precision mediump float;
in vec3 fragColor;
in float depth;
out vec4 outColor;
uniform float ledSize;
uniform float glowIntensity;
uniform float brightness;

void main() {
    // Create a circular point with soft edges
    float dist = distance(gl_PointCoord, vec2(0.5, 0.5));
    
    // Discard fragments outside the circle
    if (dist > 0.5) {
        discard;
    }
    
    // Create a bright center with soft glow
    float intensity = smoothstep(0.5, 0.0, dist);
    intensity = pow(intensity, 1.2); // Slightly softer falloff for better glow
    
    // Calculate overall brightness of the color
    float colorBrightness = max(max(fragColor.r, fragColor.g), fragColor.b);
    
    // Make the sphere visible based on global brightness
    vec3 baseColor;
    if (colorBrightness < 0.05) {
        // For very dim/off LEDs, make them visible based on global brightness
        // At high brightness (>0.8), make the sphere structure visible
        float sphereVisibility = max(0.0, (brightness - 0.8) * 5.0); // Ramp up from 0.8 to 1.0
        baseColor = mix(fragColor * 0.1, vec3(0.1, 0.1, 0.15), sphereVisibility);
    } else {
        // For lit LEDs, enhance color vibrancy and brightness
        baseColor = fragColor * 6.0; // Significantly boost colors
        baseColor = clamp(baseColor, 0.0, 1.0); // Ensure valid color range
        
        // Add a much smaller white core for lit LEDs to preserve color
        float glowAmount = pow(colorBrightness, 1.1) * 0.4; // Reduced from 0.9 to preserve colors
        baseColor = mix(baseColor, vec3(1.0), glowAmount);
    }
    
    // Add bloom/glow effect that preserves color
    float glow = glowIntensity * pow(1.0 - dist, 2.0) * colorBrightness;
    baseColor += fragColor * glow; // Use original color for glow instead of white
    baseColor = clamp(baseColor, 0.0, 1.0);
    
    // Fade out LEDs that are facing away from the camera (back-facing)
    // This helps hide LEDs that should be occluded by the model
    float depthFade = 1.0;
    if (depth < 0.0) {
        // Completely hide points behind the camera
        discard;
    } else {
        // Apply a much softer fade based on depth
        depthFade = clamp(1.0 - (depth / 8.0), 0.3, 1.0); // Using depth fade values
        
        // Only discard points that are completely facing away
        if (depthFade < 0.1) {
            discard;
        }
    }
    
    // Apply the glow effect and depth fade through the alpha channel
    outColor = vec4(baseColor, intensity * depthFade);
}
)";

WebPlatform::WebPlatform(uint16_t num_leds)
    : _num_leds(num_leds), _leds(nullptr), _brightness(DEFAULT_BRIGHTNESS), _gl_initialized(false) {
    
    try {
        // Allocate memory for LEDs
        _leds = new CRGB[num_leds];
        
        // Initialize all LEDs to bright colors for testing
        for (uint16_t i = 0; i < num_leds; i++) {
            // Create a rainbow pattern for initial testing
            uint8_t hue = (i * 255) / num_leds;
            _leds[i] = CRGB(hue, 255, 255); // Use HSV to RGB conversion internally
        }
        
        if (g_debug_mode) {
            std::cout << "WebPlatform initialized with " << num_leds << " LEDs" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in WebPlatform constructor: " << e.what() << std::endl;
        _leds = nullptr;
        _num_leds = 0;
    } catch (...) {
        std::cerr << "Unknown error in WebPlatform constructor" << std::endl;
        _leds = nullptr;
        _num_leds = 0;
    }
}

WebPlatform::~WebPlatform() {
    delete[] _leds;
    
    // Clean up WebGL resources
    if (_gl_initialized) {
        glDeleteVertexArrays(1, &_vao);
        glDeleteBuffers(1, &_vbo);
        glDeleteProgram(_shader_program);
    }
}

CRGB* WebPlatform::getLEDs() {
    return _leds;
}

uint16_t WebPlatform::getNumLEDs() const {
    return _num_leds;
}

bool WebPlatform::initWebGL() {
    if (g_debug_mode) {
        std::cout << "Initializing WebGL..." << std::endl;
    }
    
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;
    attrs.antialias = true;
    attrs.premultipliedAlpha = false;
    attrs.preserveDrawingBuffer = true;
    attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE;
    attrs.failIfMajorPerformanceCaveat = false;
    
    if (g_debug_mode) {
        std::cout << "Creating WebGL context..." << std::endl;
    }
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
    if (context <= 0) {
        std::cerr << "Failed to create WebGL context! Error code: " << context << std::endl;
        return false;
    }
    
    if (g_debug_mode) {
        std::cout << "Making WebGL context current..." << std::endl;
    }
    EMSCRIPTEN_RESULT result = emscripten_webgl_make_context_current(context);
    if (result != EMSCRIPTEN_RESULT_SUCCESS) {
        std::cerr << "Failed to make WebGL context current! Error code: " << result << std::endl;
        return false;
    }
    
    // Create and compile vertex shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);
    
    // Check vertex shader compilation
    GLint success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
        std::cerr << "Vertex shader compilation failed: " << info_log << std::endl;
        return false;
    }
    
    // Create and compile fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);
    
    // Check fragment shader compilation
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
        std::cerr << "Fragment shader compilation failed: " << info_log << std::endl;
        return false;
    }
    
    // Create shader program and link shaders
    _shader_program = glCreateProgram();
    glAttachShader(_shader_program, vertex_shader);
    glAttachShader(_shader_program, fragment_shader);
    glLinkProgram(_shader_program);
    
    // Check program linking
    glGetProgramiv(_shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(_shader_program, 512, nullptr, info_log);
        std::cerr << "Shader program linking failed: " << info_log << std::endl;
        return false;
    }
    
    // Delete shaders as they're linked into the program now
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    // Get uniform locations
    _projectionLoc = glGetUniformLocation(_shader_program, "projection");
    _viewLoc = glGetUniformLocation(_shader_program, "view");
    _pointSizeLoc = glGetUniformLocation(_shader_program, "pointSize");
    _timeLoc = glGetUniformLocation(_shader_program, "time");
    
    // Create VAO and VBO
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    
    // Bind VAO first, then bind and set VBO
    glBindVertexArray(_vao);
    
    // Set up initial buffer data (will be updated each frame)
    struct Vertex {
        float x, y, z;  // Now using 3D coordinates
        float r, g, b;
    };
    
    std::vector<Vertex> vertices(_num_leds);
    for (uint16_t i = 0; i < _num_leds; i++) {
        float angle = 2.0f * 3.14159f * i / _num_leds;
        float radius = 0.8f;
        vertices[i].x = radius * cos(angle);
        vertices[i].y = radius * sin(angle);
        vertices[i].z = 0.0f;
        vertices[i].r = 0.0f;
        vertices[i].g = 0.0f;
        vertices[i].b = 0.0f;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
    
    // Position attribute (now 3D)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Unbind VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Enable depth testing for 3D rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    return true;
}

void WebPlatform::updateVertexBuffer() {
    struct Vertex {
        float x, y, z;  // Now using 3D coordinates
        float r, g, b;
    };
    
    // Make sure we have a valid buffer before updating
    if (!_leds || _num_leds <= 0) {
        std::cerr << "Invalid LED buffer or count in WebPlatform::updateVertexBuffer()" << std::endl;
        return;
    }
    
    // Create rotation matrices for model rotation
    float cos_x = cos(_rotation_x);
    float sin_x = sin(_rotation_x);
    float cos_y = cos(_rotation_y);
    float sin_y = sin(_rotation_y);
    
    std::vector<Vertex> vertices(_num_leds);
    
    // For each LED, position it based on its index
    for (uint16_t i = 0; i < _num_leds; i++) {
        // Default to circle arrangement
        float angle = 2.0f * 3.14159f * i / _num_leds;
        float radius = 0.8f;  // Keep points within the visible area (-1 to 1)
        
        float x = 0.0f, y = 0.0f, z = 0.0f;
        
        if (_coordinate_provider) {
            // Use the coordinate provider to get 3D coordinates
            _coordinate_provider(i, x, y, z);
            
            // Scale down to fit in the visible area (-1 to 1)
            float scale = 0.0018f;  // Smaller scale to make the model more compact
            x *= scale;
            y *= scale;
            z *= scale;
        } else {
            // Fallback to circle if no coordinate provider
            x = radius * cos(angle);
            y = radius * sin(angle);
            z = 0.0f;
        }
        
        // Apply model rotation consistently regardless of view
        // First rotate around Y axis (horizontal rotation)
        float x_rotated = x * cos_y + z * sin_y;
        float z_rotated = -x * sin_y + z * cos_y;
        
        // Then rotate around X axis (vertical rotation)
        float y_final = y * cos_x - z_rotated * sin_x;
        float z_final = y * sin_x + z_rotated * cos_x;
        
        // Store rotated coordinates
        vertices[i].x = x_rotated;
        vertices[i].y = y_final;
        vertices[i].z = z_final;
        
        // Apply brightness scaling with bounds checking
        float brightness_scale = _brightness / 255.0f;
        
        // Make sure we're not accessing out of bounds
        if (i < _num_leds) {
            // Get base RGB values
            float r = _leds[i].r / 255.0f;
            float g = _leds[i].g / 255.0f;
            float b = _leds[i].b / 255.0f;
            
            // Apply brightness as an additive boost rather than a multiplier
            // This ensures colors remain vibrant even at lower brightness settings
            float boost = brightness_scale;  // Full brightness effect
            
            // Significantly increase brightness scaling and preserve color saturation
            vertices[i].r = std::min(r * brightness_scale * COLOR_BRIGHTNESS_BOOST + boost * 0.1f, 1.0f);
            vertices[i].g = std::min(g * brightness_scale * COLOR_BRIGHTNESS_BOOST + boost * 0.1f, 1.0f);
            vertices[i].b = std::min(b * brightness_scale * COLOR_BRIGHTNESS_BOOST + boost * 0.1f, 1.0f);
            
            // Ensure colors are visible (minimum brightness)
            if (r > 0 || g > 0 || b > 0) {
                // Only apply minimum brightness if the LED is not completely off
                float max_component = std::max(std::max(vertices[i].r, vertices[i].g), vertices[i].b);
                if (max_component < MIN_LED_BRIGHTNESS) {
                    // Scale up all components proportionally
                    float scale_factor = MIN_LED_BRIGHTNESS / max_component;
                    vertices[i].r *= scale_factor;
                    vertices[i].g *= scale_factor;
                    vertices[i].b *= scale_factor;
                }
                
                // Enhance color saturation by reducing the minimum component
                float min_component = std::min(std::min(vertices[i].r, vertices[i].g), vertices[i].b);
                if (min_component > 0.0f) {
                    // Reduce the minimum component to increase saturation
                    float saturation_factor = 0.7f; // Adjust to control saturation level
                    vertices[i].r -= min_component * saturation_factor;
                    vertices[i].g -= min_component * saturation_factor;
                    vertices[i].b -= min_component * saturation_factor;
                    
                    // Ensure values remain positive
                    vertices[i].r = std::max(vertices[i].r, 0.0f);
                    vertices[i].g = std::max(vertices[i].g, 0.0f);
                    vertices[i].b = std::max(vertices[i].b, 0.0f);
                }
            } else {
                // For completely off LEDs, make them very dark but still visible
                vertices[i].r = 0.02f;
                vertices[i].g = 0.02f;
                vertices[i].b = 0.02f;
            }
        } else {
            // Default color if out of bounds
            vertices[i].r = 1.0f;  // Bright red for error indication
            vertices[i].g = 0.0f;
            vertices[i].b = 0.0f;
        }
    }
    
    // Make sure WebGL is initialized before updating buffer
    if (!_gl_initialized) {
        std::cerr << "WebGL not initialized in WebPlatform::updateVertexBuffer()" << std::endl;
        return;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
}

void WebPlatform::createViewMatrix(float* view_matrix) {
    // Start with identity matrix
    for (int i = 0; i < 16; i++) {
        view_matrix[i] = 0.0f;
    }
    view_matrix[0] = 1.0f;
    view_matrix[5] = 1.0f;
    view_matrix[10] = 1.0f;
    view_matrix[15] = 1.0f;
    
    // Create a fixed camera view matrix (looking at origin from a distance)
    // Position camera at a fixed distance on the Z axis
    float translation[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, -_camera_distance, 1.0f
    };
    
    // Copy the translation matrix to the view matrix
    for (int i = 0; i < 16; i++) {
        view_matrix[i] = translation[i];
    }
    
    // Print rotation values for debugging
    if (g_debug_mode && (_rotation_x != 0.0f || _rotation_y != 0.0f)) {
        // Only print occasionally to reduce spam
        static uint32_t last_rotation_debug_time = 0;
        uint32_t current_time = emscripten_get_now();
        if (current_time - last_rotation_debug_time > 5000) { // Only print every 5 seconds
            std::cout << "Rotation: X=" << _rotation_x << ", Y=" << _rotation_y << std::endl;
            last_rotation_debug_time = current_time;
        }
    }
}

void WebPlatform::show() {
    // Initialize WebGL if not already done
    if (!_gl_initialized) {
        if (g_debug_mode) {
            std::cout << "First call to show(), initializing WebGL..." << std::endl;
        }
        if (!initWebGL()) {
            std::cerr << "Failed to initialize WebGL!" << std::endl;
            return;
        }
        _gl_initialized = true;
        if (g_debug_mode) {
            std::cout << "WebGL initialized successfully!" << std::endl;
        }
    }
    
    // Update auto-rotation if enabled
    updateAutoRotation();
    
    // Update vertex buffer with LED colors
    updateVertexBuffer();
    
    // Clear the screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Pure black background for better contrast
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Draw LEDs
    glUseProgram(_shader_program);
    glBindVertexArray(_vao);
    
    // Get canvas dimensions for correct aspect ratio
    int width, height;
    emscripten_get_canvas_element_size("#canvas", &width, &height);
    float aspect = width > 0 && height > 0 ? (float)width / (float)height : 1.0f;
    
    // Set up projection matrix (perspective)
    float fov = CAMERA_FOV_DEGREES * 3.14159f / 180.0f;  // Convert degrees to radians
    float near = CAMERA_NEAR_PLANE;
    float far = CAMERA_FAR_PLANE;
    
    // Create perspective projection matrix with correct aspect ratio
    float f = 1.0f / tan(fov / 2.0f);
    float projection[16] = {
        f / aspect, 0.0f, 0.0f, 0.0f,
        0.0f, f, 0.0f, 0.0f,
        0.0f, 0.0f, (far + near) / (near - far), -1.0f,
        0.0f, 0.0f, (2.0f * far * near) / (near - far), 0.0f
    };
    
    // Create view matrix with rotation
    float view[16];
    createViewMatrix(view);
    
    // Set uniforms
    if (_projectionLoc != -1) {
        glUniformMatrix4fv(_projectionLoc, 1, GL_FALSE, projection);
    }
    
    if (_viewLoc != -1) {
        glUniformMatrix4fv(_viewLoc, 1, GL_FALSE, view);
    }
    
    if (_pointSizeLoc != -1) {
        glUniform1f(_pointSizeLoc, _led_size);  // Use the configurable LED size
    }
    
    // Set the new uniforms for LED size and glow intensity
    GLint ledSizeLoc = glGetUniformLocation(_shader_program, "ledSize");
    if (ledSizeLoc != -1) {
        glUniform1f(ledSizeLoc, _led_size);
    }
    
    GLint glowIntensityLoc = glGetUniformLocation(_shader_program, "glowIntensity");
    if (glowIntensityLoc != -1) {
        glUniform1f(glowIntensityLoc, _glow_intensity);
    }
    
    // Pass the brightness as a uniform for sphere visibility
    GLint brightnessLoc = glGetUniformLocation(_shader_program, "brightness");
    if (brightnessLoc != -1) {
        glUniform1f(brightnessLoc, _brightness / 255.0f);
    }
    
    if (_timeLoc != -1) {
        // Pass current time for animation (not used for rotation anymore)
        glUniform1f(_timeLoc, emscripten_get_now());
    }
    
    // Print debug info about what we're drawing
    if (_num_leds > 0 && g_debug_mode) {
        // Only print occasionally to reduce spam
        static uint32_t last_debug_time = 0;
        uint32_t current_time = emscripten_get_now();
        if (current_time - last_debug_time > 10000) { // Only print every 10 seconds
            std::cout << "Drawing " << _num_leds << " LEDs. Brightness: " << (int)_brightness << std::endl;
            last_debug_time = current_time;
        }
    }
    
    // Enable backface culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    glDrawArrays(GL_POINTS, 0, _num_leds);
    
    // Disable culling after drawing
    glDisable(GL_CULL_FACE);
    
    // Unbind
    glBindVertexArray(0);
    glUseProgram(0);
}

void WebPlatform::setBrightness(uint8_t brightness) {
    // Only log brightness changes in debug mode
    if (_brightness != brightness && g_debug_mode) {
        std::cout << "Brightness changed from " << (int)_brightness << " to " << (int)brightness << std::endl;
    }
    _brightness = brightness;
}

void WebPlatform::clear() {
    for (uint16_t i = 0; i < _num_leds; i++) {
        _leds[i] = CRGB(0, 0, 0);
    }
}

void WebPlatform::setMaxRefreshRate(uint8_t fps) {
    _max_refresh_rate = fps;
    // Note: In WebGL, we don't control the refresh rate directly
    // It's tied to the browser's requestAnimationFrame
}

void WebPlatform::setDither(uint8_t dither) {
    _dither = dither;
    // Note: Dithering would need to be implemented in the shader
    // For simplicity, we're ignoring this for now
}

void WebPlatform::setLEDSize(float size) {
    _led_size = size;
}

float WebPlatform::getLEDSize() const {
    return _led_size;
}

void WebPlatform::setGlowIntensity(float intensity) {
    _glow_intensity = intensity;
}

float WebPlatform::getGlowIntensity() const {
    return _glow_intensity;
}

void WebPlatform::setLEDSpacing(float spacing) {
    _led_spacing = spacing;
}

void WebPlatform::setLEDArrangement(const float* positions, uint16_t count) {
    if (positions == nullptr || count == 0) {
        _custom_arrangement = false;
        return;
    }
    
    _custom_arrangement = true;
    
    // Clean up existing positions if any
    delete[] _led_positions;
    
    // Allocate and copy new positions
    _led_positions = new float[count * 3]; // x, y, z for each LED
    std::memcpy(_led_positions, positions, count * 3 * sizeof(float));
}

GLuint WebPlatform::compileShader(unsigned int type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    // Check compilation status
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, nullptr, info_log);
        std::cerr << "Shader compilation failed: " << info_log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

} // namespace PixelTheater

#else
// =========================================================================
// MINIMAL STUB IMPLEMENTATION FOR NON-WEB BUILDS
// =========================================================================

#include <iostream>

namespace PixelTheater {

WebPlatform::WebPlatform(uint16_t num_leds) : _num_leds(num_leds) {
    _leds = new CRGB[num_leds]();  // Initialize to zero
    std::cout << "Created stub WebPlatform with " << num_leds << " LEDs (non-web build)" << std::endl;
}

WebPlatform::~WebPlatform() {
    delete[] _leds;
}

CRGB* WebPlatform::getLEDs() {
    return _leds;
}

uint16_t WebPlatform::getNumLEDs() const {
    return _num_leds;
}

void WebPlatform::show() {
    // Do nothing in non-web builds
}

void WebPlatform::setBrightness(uint8_t brightness) {
    _brightness = brightness;
}

void WebPlatform::clear() {
    for (uint16_t i = 0; i < _num_leds; i++) {
        _leds[i] = CRGB(0, 0, 0);
    }
}

void WebPlatform::setMaxRefreshRate(uint8_t fps) {
    _max_refresh_rate = fps;
}

void WebPlatform::setDither(uint8_t dither) {
    _dither = dither;
}

} // namespace PixelTheater

#endif // PLATFORM_WEB || EMSCRIPTEN 