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
#include <numeric>

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

// Vertex shader source with 3D projection - First pass
const char* vertex_shader_source = R"(#version 300 es
precision mediump float;
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
out vec3 fragColor;
out float depth;
out float occlusion; // Pass occlusion to fragment shader
uniform float ledSizeRatio;  // User-controlled size ratio (0.2 to 2.5)
uniform float physicalLedSize; // Physical LED size in model units
uniform float cameraDistance;  // Current camera distance
uniform mat4 projection;  // Projection matrix
uniform mat4 view;        // View matrix
uniform float time;       // Time for animation
uniform float canvasHeight; // Height of the canvas in pixels

void main() {
    // Apply transformations
    vec4 viewPos = view * vec4(position, 1.0);
    gl_Position = projection * viewPos;
    
    // Calculate depth for fragment shader (to fade distant points)
    depth = -viewPos.z;
    
    // The occlusion is determined by the color sum:
    // If all color components are very near zero, the LED is occluded
    occlusion = (color.r + color.g + color.b) < 0.01 ? 0.0 : 1.0;
    
    // Calculate physically-based LED size
    // 1. Base physical size calculation
    float physicalSize = physicalLedSize;
    
    // 2. Apply perspective division to get size in screen space
    // The negative viewPos.z is the distance to the camera in view space
    float distanceToCamera = -viewPos.z;
    
    // 3. Calculate a size that scales properly with distance
    // This is based on similar triangles principle in a perspective projection
    float fieldOfViewFactor = canvasHeight / (2.0 * distanceToCamera * tan(radians(15.0))); // 15 degrees is half the FOV
    float baseScreenSize = physicalSize * fieldOfViewFactor;
    
    // 4. Apply user's size ratio
    float finalSize = baseScreenSize * ledSizeRatio;
    
    // 5. Ensure a minimum visible size and apply very subtle distance-based scaling
    float distanceScale = 1.0 - gl_Position.z * 0.05; // Gentler scaling factor
    gl_PointSize = max(finalSize * distanceScale, 3.0); // Minimum visible size
    
    fragColor = color;
}
)";

// Fragment shader source - First pass (LED rendering)
const char* fragment_shader_source = R"(#version 300 es
precision mediump float;
in vec3 fragColor;
in float depth;
in float occlusion; // Receive occlusion from vertex shader
out vec4 outColor;
uniform float brightness;

void main() {
    // If the LED is occluded, discard the fragment immediately
    if (occlusion < 0.1) {
        discard;
    }
    
    // Distance from center of the point
    float dist = distance(gl_PointCoord, vec2(0.5, 0.5));
    
    // Simple LED with a bright core and smooth falloff
    if (dist > 0.5) {
        discard; // Outside the LED's circle
    }
    
    // Calculate normalized brightness (0.0-1.0)
    float normalizedBrightness = brightness / 255.0;
    
    // Calculate overall brightness of the color
    float colorBrightness = max(max(fragColor.r, fragColor.g), fragColor.b);
    
    // For lit LEDs
    vec3 baseColor;
    float alpha;
    
    if (colorBrightness > 0.01) {
        // Core brightness with sharper center
        float coreBrightness = 1.0 - pow(dist * 1.8, 2.0);
        
        // Apply color with more moderate intensity
        // Scale down with brightness to preserve dynamic range
        baseColor = fragColor * 2.5 * coreBrightness;
        
        // Add white highlight at center for extra brightness
        float centerHighlight = 1.0 - dist * 3.5;
        centerHighlight = max(0.0, centerHighlight);
        baseColor = mix(baseColor, vec3(1.0), centerHighlight * 0.2);
        
        // Use higher alpha for brighter LEDs
        alpha = coreBrightness;
    } 
    // For unlit LEDs - show model structure at very high brightness or when brightness is 0
    else {
        if (normalizedBrightness > 0.90) {
            // Make unlit LEDs faintly visible at high brightness to see model structure
            float modelVisibility = (normalizedBrightness - 0.90) * 10.0; // Ramp up from 90% to 100%
            baseColor = vec3(0.15, 0.15, 0.2) * modelVisibility;
            alpha = modelVisibility * 0.5;
        } 
        else if (normalizedBrightness < 0.01) {
            // When brightness is essentially zero, ensure LEDs are completely dark
            baseColor = vec3(0.0);
            alpha = 0.0;
            discard; // Skip rendering at zero brightness
        }
        else {
            // Otherwise discard unlit LEDs
            discard;
        }
    }
    
    // Apply depth fading
    float depthFade = 1.0;
    if (depth < 0.0) {
        discard; // Behind the camera
    } else {
        depthFade = clamp(1.0 - (depth / 8.0), 0.3, 1.0);
        if (depthFade < 0.1) {
            discard; // Too far away
        }
    }
    
    // Apply depth fade and occlusion to color (not just alpha)
    // This preserves more brightness while still applying falloff
    baseColor *= depthFade * occlusion;
    
    // No additional boost needed with additive blending
    
    // Clamp color values
    baseColor = clamp(baseColor, 0.0, 1.0);
    
    // Output color with full alpha for bright pixels, partial for dim ones
    // This preserves brightness better with the blending mode we'll use
    alpha = clamp(max(max(baseColor.r, baseColor.g), baseColor.b), 0.0, 1.0);
    outColor = vec4(baseColor, alpha);
}
)";

// Vertex shader for post-processing (screen quad)
const char* quad_vertex_shader_source = R"(#version 300 es
precision mediump float;
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;
out vec2 fragTexCoord;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    fragTexCoord = texCoord;
}
)";

// Fragment shader for bloom post-processing
const char* bloom_fragment_shader_source = R"(#version 300 es
precision mediump float;
in vec2 fragTexCoord;
out vec4 outColor;
uniform sampler2D sceneTex;
uniform float bloomIntensity;

void main() {
    // Sample the original scene
    vec4 originalColor = texture(sceneTex, fragTexCoord);
    
    // Always preserve the original brightness
    vec3 baseColor = originalColor.rgb;
    
    // Skip bloom processing for very low bloom settings
    if (bloomIntensity <= 0.05) {
        outColor = vec4(baseColor, originalColor.a);
        return;
    }
    
    // Use a larger sampling area for higher bloom settings
    // Scale up for extended range (max is now 3.0)
    float bloomRadius = max(0.5, bloomIntensity * 1.5);
    vec4 bloomAccumulator = vec4(0.0);
    float totalWeight = 0.0;
    
    // Sample in a larger radius for bloom effect
    const int SAMPLES = 16;
    for (int i = 0; i < SAMPLES; i++) {
        float angle = float(i) * (3.14159 * 2.0) / float(SAMPLES);
        vec2 offset = vec2(cos(angle), sin(angle)) * bloomRadius / vec2(textureSize(sceneTex, 0));
        
        // Weight based on distance from center
        float weight = 1.0 - float(i) / float(SAMPLES);
        vec4 sampleColor = texture(sceneTex, fragTexCoord + offset);
        
        // Add weighted sample to accumulator
        bloomAccumulator += sampleColor * weight;
        totalWeight += weight;
    }
    
    // Normalize the bloom color
    vec4 bloomColor = bloomAccumulator / max(totalWeight, 0.001);
    
    // Apply bloom with intensity scaling that allows for higher values
    // Scale up for the extended slider range (0-3.0)
    float scaledIntensity = bloomIntensity * 0.7;
    vec3 bloomResult = bloomColor.rgb * scaledIntensity;
    
    // Combine original scene with bloom (additive)
    vec3 finalColor = baseColor + bloomResult;
    
    // Preserve alpha from original scene
    outColor = vec4(finalColor, originalColor.a);
}
)";

WebPlatform::WebPlatform(uint16_t num_leds)
    : _num_leds(num_leds), _leds(nullptr), _brightness(DEFAULT_BRIGHTNESS), 
      _led_size(DEFAULT_LED_SIZE), _bloom_intensity(DEFAULT_BLOOM_INTENSITY), _gl_initialized(false) {
    
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
    delete[] _led_positions;
    
    // Clean up WebGL resources
    if (_gl_initialized) {
        glDeleteVertexArrays(1, &_vao);
        glDeleteBuffers(1, &_vbo);
        glDeleteProgram(_shader_program);
        glDeleteProgram(_bloom_shader_program);
        
        // Delete framebuffer resources
        glDeleteFramebuffers(1, &_scene_fbo);
        glDeleteTextures(1, &_scene_texture);
        glDeleteRenderbuffers(1, &_scene_depth_rbo);
        
        // Delete quad resources
        glDeleteVertexArrays(1, &_quad_vao);
        glDeleteBuffers(1, &_quad_vbo);
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
    
    // Get canvas dimensions for framebuffer setup
    emscripten_get_canvas_element_size("#canvas", &_canvas_width, &_canvas_height);
    
    // Create and compile main shader program (LED rendering)
    _shader_program = createShaderProgram(vertex_shader_source, fragment_shader_source);
    if (!_shader_program) {
        return false;
    }
    
    // Create and compile bloom shader program (post-processing)
    _bloom_shader_program = createShaderProgram(quad_vertex_shader_source, bloom_fragment_shader_source);
    if (!_bloom_shader_program) {
        return false;
    }
    
    // Get uniform locations for main shader
    _projectionLoc = glGetUniformLocation(_shader_program, "projection");
    _viewLoc = glGetUniformLocation(_shader_program, "view");
    _timeLoc = glGetUniformLocation(_shader_program, "time");
    
    // Create VAO and VBO for LEDs
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    
    // Bind VAO first, then bind and set VBO
    glBindVertexArray(_vao);
    
    // Set up initial buffer data (will be updated each frame)
    struct Vertex {
        float x, y, z;  // 3D coordinates
        float r, g, b;  // Color
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
    
    // Position attribute (3D)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Create a screen-aligned quad for the post-processing pass
    glGenVertexArrays(1, &_quad_vao);
    glGenBuffers(1, &_quad_vbo);
    
    glBindVertexArray(_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _quad_vbo);
    
    // Define the quad vertices (full screen)
    float quadVertices[] = {
        // positions   // texture coords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinates attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Set up framebuffers for multi-pass rendering
    setupFramebuffers();
    
    // Enable depth testing and blending
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    
    // Use additive blending for LED points - much brighter results
    // Source color is added to destination color (GL_ONE, GL_ONE)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    return true;
}

GLuint WebPlatform::createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);
    
    // Check vertex shader compilation
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
        return 0;
    }
    
    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);
    
    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
        return 0;
    }
    
    // Create and link shader program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // Check program linking
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        return 0;
    }
    
    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

void WebPlatform::setupFramebuffers() {
    // Create framebuffer for the scene render
    glGenFramebuffers(1, &_scene_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _scene_fbo);
    
    // Create texture for color attachment
    glGenTextures(1, &_scene_texture);
    glBindTexture(GL_TEXTURE_2D, _scene_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _canvas_width, _canvas_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _scene_texture, 0);
    
    // Create renderbuffer for depth attachment
    glGenRenderbuffers(1, &_scene_depth_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, _scene_depth_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _canvas_width, _canvas_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _scene_depth_rbo);
    
    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
    }
    
    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    
    // First, collect all coordinates to calculate the model center
    std::vector<float> x_coords, y_coords, z_coords;
    x_coords.reserve(_num_leds);
    y_coords.reserve(_num_leds);
    z_coords.reserve(_num_leds);
    
    // Collect all LED coordinates
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
        
        x_coords.push_back(x);
        y_coords.push_back(y);
        z_coords.push_back(z);
    }
    
    // Calculate model center (average of all LED positions)
    float center_x = 0.0f, center_y = 0.0f, center_z = 0.0f;
    if (!x_coords.empty()) {
        center_x = std::accumulate(x_coords.begin(), x_coords.end(), 0.0f) / x_coords.size();
        center_y = std::accumulate(y_coords.begin(), y_coords.end(), 0.0f) / y_coords.size();
        center_z = std::accumulate(z_coords.begin(), z_coords.end(), 0.0f) / z_coords.size();
    }
    
    if (g_debug_mode && frame_count % 600 == 0) {
        std::cout << "Model center: " << center_x << ", " << center_y << ", " << center_z << std::endl;
    }
    
    // For each LED, position it based on its index
    for (uint16_t i = 0; i < _num_leds; i++) {
        float x = x_coords[i];
        float y = y_coords[i];
        float z = z_coords[i];
        
        // Apply model rotation consistently regardless of view
        // First rotate around Y axis (horizontal rotation)
        float x_rotated = x * cos_y + z * sin_y;
        float z_rotated = -x * sin_y + z * cos_y;
        
        // Then rotate around X axis (vertical rotation)
        float y_final = y * cos_x - z_rotated * sin_x;
        float z_final = y * sin_x + z_rotated * cos_x;
        
        // Also rotate the center point
        float center_x_rotated = center_x * cos_y + center_z * sin_y;
        float center_z_rotated = -center_x * sin_y + center_z * cos_y;
        float center_y_final = center_y * cos_x - center_z_rotated * sin_x;
        float center_z_final = center_y * sin_x + center_z_rotated * cos_x;
        
        // Store rotated coordinates
        vertices[i].x = x_rotated;
        vertices[i].y = y_final;
        vertices[i].z = z_final;
        
        // Check if this LED is visible from the camera viewpoint
        // Camera is at (0, 0, -_camera_distance) looking at the origin
        
        // Vector from camera to center of model
        float cam_to_center_x = center_x_rotated;
        float cam_to_center_y = center_y_final;
        float cam_to_center_z = center_z_final + _camera_distance; // Camera is at -_camera_distance
        
        // Vector from camera to this LED
        float cam_to_led_x = x_rotated;
        float cam_to_led_y = y_final;
        float cam_to_led_z = z_final + _camera_distance;
        
        // Vector from center to this LED
        float center_to_led_x = x_rotated - center_x_rotated;
        float center_to_led_y = y_final - center_y_final;
        float center_to_led_z = z_final - center_z_final;
        
        // Dot product of camera-to-center and center-to-LED
        // If positive, LED is on the far side of the center from the camera's viewpoint
        float dot_product = (cam_to_center_x * center_to_led_x +
                             cam_to_center_y * center_to_led_y +
                             cam_to_center_z * center_to_led_z);
        
        // Distance from center to LED
        float center_to_led_distance = std::sqrt(
            center_to_led_x * center_to_led_x +
            center_to_led_y * center_to_led_y +
            center_to_led_z * center_to_led_z
        );
        
        // Calculate occlusion factor (0=hidden, 1=fully visible)
        float occlusion_factor = 1.0f;
        
        // If the LED is on the far side from the camera (using the center as reference),
        // then apply occlusion. A negative dot product means the LED is on the opposite
        // side of the center from the camera's viewpoint.
        if (dot_product < 0 && center_to_led_distance > 0.03f) {
            // Apply a smooth transition near the edge of visibility
            // The more negative the dot product, the more it's hidden
            float hide_threshold = -0.2f * center_to_led_distance;
            if (dot_product < hide_threshold) {
                occlusion_factor = 0.0f; // Completely hidden
            } else {
                // Smooth transition at the edges
                occlusion_factor = 1.0f - (dot_product / hide_threshold);
            }
        }
        
        // Make sure we're not accessing out of bounds
        if (i < _num_leds) {
            // Get base RGB values
            float r = _leds[i].r / 255.0f;
            float g = _leds[i].g / 255.0f;
            float b = _leds[i].b / 255.0f;
            
            // Check for zero brightness to avoid white LEDs at brightness=0
            if (_brightness < 2) {
                vertices[i].r = 0.0f;
                vertices[i].g = 0.0f;
                vertices[i].b = 0.0f;
            } else {
                // Apply enhanced brightness scaling with better dynamic range preservation
                float brightness_scale = _brightness / 255.0f;
                
                // Use a more moderate multiplier for better dynamic range
                float multiplier = 1.5f;
                float boost = brightness_scale * 0.05f; // Very small boost
                
                vertices[i].r = std::min(r * brightness_scale * multiplier + boost, 1.0f);
                vertices[i].g = std::min(g * brightness_scale * multiplier + boost, 1.0f);
                vertices[i].b = std::min(b * brightness_scale * multiplier + boost, 1.0f);
                
                // Apply occlusion factor (0=hidden, 1=fully visible)
                vertices[i].r *= occlusion_factor;
                vertices[i].g *= occlusion_factor;
                vertices[i].b *= occlusion_factor;
                
                // Ensure colors are visible (minimum brightness)
                if ((r > 0 || g > 0 || b > 0) && occlusion_factor > 0.0f) {
                    // Only apply minimum brightness if the LED is not completely off and not occluded
                    float max_component = std::max(std::max(vertices[i].r, vertices[i].g), vertices[i].b);
                    float min_brightness = MIN_LED_BRIGHTNESS;
                    
                    if (max_component < min_brightness) {
                        // Scale up all components proportionally
                        float scale_factor = min_brightness / max_component;
                        vertices[i].r *= scale_factor;
                        vertices[i].g *= scale_factor;
                        vertices[i].b *= scale_factor;
                    }
                    
                    // Enhance color saturation by reducing the minimum component
                    float min_component = std::min(std::min(vertices[i].r, vertices[i].g), vertices[i].b);
                    if (min_component > 0.0f) {
                        // Reduce the minimum component to increase saturation
                        float saturation_factor = 0.3f; // Lower value preserves more accurate colors
                        vertices[i].r -= min_component * saturation_factor;
                        vertices[i].g -= min_component * saturation_factor;
                        vertices[i].b -= min_component * saturation_factor;
                        
                        // Ensure values remain positive
                        vertices[i].r = std::max(vertices[i].r, 0.0f);
                        vertices[i].g = std::max(vertices[i].g, 0.0f);
                        vertices[i].b = std::max(vertices[i].b, 0.0f);
                    }
                } 
                // For unlit LEDs
                else if (occlusion_factor > 0.0f && _brightness > 230) {
                    // At very high brightness, visualize unlit LEDs to see the model structure
                    float visibility = (_brightness - 230.0f) / 25.0f; // Ramp up from 230-255
                    vertices[i].r = 0.03f * visibility * occlusion_factor;
                    vertices[i].g = 0.03f * visibility * occlusion_factor;
                    vertices[i].b = 0.05f * visibility * occlusion_factor; // Slightly blue tint
                } else {
                    // Completely off LEDs or occluded LEDs are invisible
                    vertices[i].r = 0.0f;
                    vertices[i].g = 0.0f;
                    vertices[i].b = 0.0f;
                }
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

void WebPlatform::firstPass() {
    // Render scene to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, _scene_fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Clear with transparent black
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Use the main shader for LED rendering
    glUseProgram(_shader_program);
    
    // Calculate and set projection matrix
    float fov = CAMERA_FOV_DEGREES * 3.14159f / 180.0f;
    float aspect = static_cast<float>(_canvas_width) / static_cast<float>(_canvas_height);
    float f = 1.0f / tan(fov / 2.0f);
    float projection[16] = {
        f / aspect, 0.0f, 0.0f, 0.0f,
        0.0f, f, 0.0f, 0.0f,
        0.0f, 0.0f, (CAMERA_FAR_PLANE + CAMERA_NEAR_PLANE) / (CAMERA_NEAR_PLANE - CAMERA_FAR_PLANE), -1.0f,
        0.0f, 0.0f, (2.0f * CAMERA_FAR_PLANE * CAMERA_NEAR_PLANE) / (CAMERA_NEAR_PLANE - CAMERA_FAR_PLANE), 0.0f
    };
    
    // Create and set view matrix
    float view[16];
    createViewMatrix(view);
    
    // Set uniforms
    glUniformMatrix4fv(_projectionLoc, 1, GL_FALSE, projection);
    glUniformMatrix4fv(_viewLoc, 1, GL_FALSE, view);
    
    // Set new physically-based LED size uniforms
    GLint ledSizeRatioLoc = glGetUniformLocation(_shader_program, "ledSizeRatio");
    if (ledSizeRatioLoc != -1) {
        glUniform1f(ledSizeRatioLoc, _led_size);
    }
    
    // The physical LED size in model units
    // Convert from mm to model units based on the scaling factor used in the model
    // Scaling factor is 0.0018f, as used in updateVertexBuffer
    GLint physicalLedSizeLoc = glGetUniformLocation(_shader_program, "physicalLedSize");
    if (physicalLedSizeLoc != -1) {
        // Calculate physical LED size relative to model scale
        float ledSizeInModelUnits = PHYSICAL_LED_DIAMETER / PHYSICAL_FACE_EDGE * 0.5f;
        glUniform1f(physicalLedSizeLoc, ledSizeInModelUnits);
    }
    
    // Pass camera distance for size calculation
    GLint cameraDistanceLoc = glGetUniformLocation(_shader_program, "cameraDistance");
    if (cameraDistanceLoc != -1) {
        glUniform1f(cameraDistanceLoc, _camera_distance);
    }
    
    // Pass canvas height for FOV calculation
    GLint canvasHeightLoc = glGetUniformLocation(_shader_program, "canvasHeight");
    if (canvasHeightLoc != -1) {
        glUniform1f(canvasHeightLoc, static_cast<float>(_canvas_height));
    }
    
    // Pass brightness uniform
    GLint brightnessLoc = glGetUniformLocation(_shader_program, "brightness");
    if (brightnessLoc != -1) {
        glUniform1f(brightnessLoc, static_cast<float>(_brightness));
    }
    
    // Draw the LEDs
    glBindVertexArray(_vao);
    glDrawArrays(GL_POINTS, 0, _num_leds);
    
    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void WebPlatform::bloomPass() {
    // Render to default framebuffer (screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Use bloom shader for post-processing
    glUseProgram(_bloom_shader_program);
    
    // Bind the scene texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _scene_texture);
    
    // Set uniforms for bloom shader
    GLint sceneTexLoc = glGetUniformLocation(_bloom_shader_program, "sceneTex");
    if (sceneTexLoc != -1) {
        glUniform1i(sceneTexLoc, 0); // Texture unit 0
    }
    
    GLint bloomIntensityLoc = glGetUniformLocation(_bloom_shader_program, "bloomIntensity");
    if (bloomIntensityLoc != -1) {
        glUniform1f(bloomIntensityLoc, _bloom_intensity);
    }
    
    // Switch to additive blending for the bloom pass
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    // Draw the full-screen quad
    glBindVertexArray(_quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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
    
    // Increment frame counter
    frame_count++;
    
    // Update auto-rotation if enabled
    updateAutoRotation();
    
    // Update vertex buffer with LED colors
    updateVertexBuffer();
    
    // Check if canvas size has changed
    int width, height;
    emscripten_get_canvas_element_size("#canvas", &width, &height);
    if (width != _canvas_width || height != _canvas_height) {
        _canvas_width = width;
        _canvas_height = height;
        
        // Resize framebuffer textures
        glBindTexture(GL_TEXTURE_2D, _scene_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _canvas_width, _canvas_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        
        glBindRenderbuffer(GL_RENDERBUFFER, _scene_depth_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _canvas_width, _canvas_height);
    }
    
    // First pass: render LEDs to framebuffer
    firstPass();
    
    // Second pass: apply bloom and render to screen
    bloomPass();
}

void WebPlatform::setBrightness(uint8_t brightness) {
    if (_brightness != brightness && g_debug_mode) {
        std::cout << "Brightness changed from " << (int)_brightness << " to " << (int)brightness << std::endl;
    }
    _brightness = brightness;
}

uint8_t WebPlatform::getBrightness() const {
    return _brightness;
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

void WebPlatform::setLEDSize(float size) {
    _led_size = size;
}

float WebPlatform::getLEDSize() const {
    return _led_size;
}

void WebPlatform::setBloomIntensity(float intensity) {
    _bloom_intensity = intensity;
}

float WebPlatform::getBloomIntensity() const {
    return _bloom_intensity;
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

} // namespace PixelTheater

#else
// =========================================================================
// MINIMAL STUB IMPLEMENTATION FOR NON-WEB BUILDS
// =========================================================================

#include <iostream>

namespace PixelTheater {

WebPlatform::WebPlatform(uint16_t num_leds) : _num_leds(num_leds) {
    _leds = new CRGB[num_leds]();  // Initialize to zero
    _brightness = DEFAULT_BRIGHTNESS;
    _led_size = DEFAULT_LED_SIZE;
    _bloom_intensity = DEFAULT_BLOOM_INTENSITY;
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