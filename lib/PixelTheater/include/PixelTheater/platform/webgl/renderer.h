#pragma once

#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)

#include <cstdint>
#include <cstddef>
#include <vector>
#include <emscripten/html5.h>

namespace PixelTheater {

class WebGLRenderer {
public:
    WebGLRenderer();
    ~WebGLRenderer();
    
    // Initialize WebGL context and resources
    bool initialize(int canvas_width, int canvas_height);
    
    // Load and compile shaders
    uint32_t createShaderProgram(const char* vertexSource, const char* fragmentSource);
    
    // Set up framebuffers for post-processing
    void setupFramebuffers(int width, int height);
    
    // Update viewport if canvas size changes
    void updateViewport(int width, int height);
    
    // Create and set up buffer objects 
    uint32_t createBuffer();
    void bindArrayBuffer(uint32_t buffer, const void* data, size_t size, bool dynamic = false);
    void bindElementBuffer(uint32_t buffer, const void* data, size_t size);
    
    // Create and set up vertex arrays
    uint32_t createVertexArray();
    void configureVertexAttributes(uint32_t vao, uint32_t vbo, bool has_normals = false);
    
    // Rendering passes
    void beginRenderPass();
    void endRenderPass();
    
    // Apply post-processing effects
    void applyPostProcessing(uint32_t glow_shader, float atmosphere_intensity);
    
    // Clean up resources
    void cleanup();
    
private:
    // Helper method to create a fullscreen quad
    void renderFullscreenQuad();
    
    // Helper method to clean up framebuffers
    void cleanupFramebuffers();
    
    // WebGL state
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE _context;
    bool _initialized;
    
    // Shader programs
    uint32_t _blur_shader;
    uint32_t _composite_shader;
    
    // Framebuffer objects
    uint32_t _scene_fbo;
    uint32_t _scene_texture;
    uint32_t _scene_depth_rbo;
    
    uint32_t _glow_fbo;
    uint32_t _glow_texture;
    uint32_t _blur_fbo;
    uint32_t _blur_texture;
    
    // Canvas dimensions
    int _canvas_width;
    int _canvas_height;
};

} // namespace PixelTheater 

#endif // defined(PLATFORM_WEB) || defined(EMSCRIPTEN) 