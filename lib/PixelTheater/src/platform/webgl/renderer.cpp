#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)

#include "PixelTheater/platform/webgl/renderer.h"
#include "PixelTheater/platform/webgl/shaders.h"
#include <emscripten.h>
#include <GLES3/gl3.h>
#include <cstdio>

namespace PixelTheater {

WebGLRenderer::WebGLRenderer() 
    : _context(0), _initialized(false), _scene_fbo(0), _scene_texture(0), 
      _scene_depth_rbo(0), _glow_fbo(0), _glow_texture(0), _blur_fbo(0), 
      _blur_texture(0), _blur_shader(0), _composite_shader(0),
      _canvas_width(0), _canvas_height(0) {
}

WebGLRenderer::~WebGLRenderer() {
    cleanup();
}

bool WebGLRenderer::initialize(int canvas_width, int canvas_height) {
    _canvas_width = canvas_width;
    _canvas_height = canvas_height;
    
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;
    attrs.alpha = true;
    attrs.depth = true;
    attrs.stencil = false;
    attrs.antialias = true;
    attrs.premultipliedAlpha = false;
    attrs.preserveDrawingBuffer = false;
    attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_DEFAULT;
    attrs.failIfMajorPerformanceCaveat = false;
    
    _context = emscripten_webgl_create_context("#canvas", &attrs);
    
    if (_context <= 0) {
        printf("Failed to create WebGL context\n");
        return false;
    }
    
    emscripten_webgl_make_context_current(_context);
    
    glViewport(0, 0, _canvas_width, _canvas_height);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Create shader programs
    printf("Compiling shaders...\n");
    _blur_shader = createShaderProgram(blur_vertex_shader_source, blur_fragment_shader_source);
    _composite_shader = createShaderProgram(quad_vertex_shader_source, composite_fragment_shader_source);
    
    setupFramebuffers(_canvas_width, _canvas_height);
    
    _initialized = true;
    return true;
}

uint32_t WebGLRenderer::createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    // Print only a short message instead of shader source
    printf("Compiling shaders...\n");
    
    // Create shader program
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    
    // Check vertex shader compilation
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
        return 0;
    }
    
    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    
    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
        return 0;
    }
    
    // Link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // Check linking
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        return 0;
    }
    
    // Clean up shader objects
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

void WebGLRenderer::setupFramebuffers(int width, int height) {
    // Clean up existing framebuffers
    cleanupFramebuffers();
    
    // Create framebuffer for scene rendering
    glGenFramebuffers(1, &_scene_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _scene_fbo);
    
    // Create color attachment texture
    glGenTextures(1, &_scene_texture);
    glBindTexture(GL_TEXTURE_2D, _scene_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _scene_texture, 0);
    
    // Create renderbuffer for depth
    glGenRenderbuffers(1, &_scene_depth_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, _scene_depth_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _scene_depth_rbo);
    
    // Create glow extraction framebuffer
    glGenFramebuffers(1, &_glow_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _glow_fbo);
    
    // Create glow texture
    glGenTextures(1, &_glow_texture);
    glBindTexture(GL_TEXTURE_2D, _glow_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _glow_texture, 0);
    
    // Create blur framebuffer
    glGenFramebuffers(1, &_blur_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _blur_fbo);
    
    // Create blur texture
    glGenTextures(1, &_blur_texture);
    glBindTexture(GL_TEXTURE_2D, _blur_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _blur_texture, 0);
    
    // Check all framebuffers
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void WebGLRenderer::cleanupFramebuffers() {
    if (_scene_fbo != 0) {
        glDeleteFramebuffers(1, &_scene_fbo);
        glDeleteTextures(1, &_scene_texture);
        glDeleteRenderbuffers(1, &_scene_depth_rbo);
    }
    if (_glow_fbo != 0) {
        glDeleteFramebuffers(1, &_glow_fbo);
        glDeleteTextures(1, &_glow_texture);
    }
    if (_blur_fbo != 0) {
        glDeleteFramebuffers(1, &_blur_fbo);
        glDeleteTextures(1, &_blur_texture);
    }
}

void WebGLRenderer::updateViewport(int width, int height) {
    _canvas_width = width;
    _canvas_height = height;
    glViewport(0, 0, width, height);
    setupFramebuffers(width, height);
}

uint32_t WebGLRenderer::createBuffer() {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    return buffer;
}

void WebGLRenderer::bindArrayBuffer(uint32_t buffer, const void* data, size_t size, bool dynamic) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

void WebGLRenderer::bindElementBuffer(uint32_t buffer, const void* data, size_t size) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

uint32_t WebGLRenderer::createVertexArray() {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    return vao;
}

void WebGLRenderer::configureVertexAttributes(uint32_t vao, uint32_t vbo, bool has_normals) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    if (has_normals) {
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    } else {
        // For LED vertices: position (xyz) and color (rgb)
        // Position attribute (xyz)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Color attribute (rgb)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    
    // Unbind the VAO to avoid accidental modification
    glBindVertexArray(0);
}

void WebGLRenderer::beginRenderPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, _scene_fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void WebGLRenderer::endRenderPass() {
    // We're done rendering to the scene FBO, but we don't want to clear the default framebuffer yet
    // The applyPostProcessing method will handle copying the scene to the default framebuffer
}

void WebGLRenderer::applyPostProcessing(uint32_t glow_shader, float atmosphere_intensity) {
    // 1. Extract bright areas
    glBindFramebuffer(GL_FRAMEBUFFER, _glow_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(glow_shader);
    
    // Set uniforms for glow extraction
    GLint sceneTexLoc = glGetUniformLocation(glow_shader, "scene_texture");
    GLint intensityLoc = glGetUniformLocation(glow_shader, "atmosphere_intensity");
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _scene_texture);
    if (sceneTexLoc >= 0) glUniform1i(sceneTexLoc, 0);
    if (intensityLoc >= 0) glUniform1f(intensityLoc, atmosphere_intensity);
    
    // Render fullscreen quad to extract bright areas
    renderFullscreenQuad();
    
    // 2. Apply horizontal blur
    glBindFramebuffer(GL_FRAMEBUFFER, _blur_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(_blur_shader);  // Use dedicated blur shader
    
    GLint directionLoc = glGetUniformLocation(_blur_shader, "blur_direction");
    GLint radiusLoc = glGetUniformLocation(_blur_shader, "blur_radius");
    GLint sourceTexLoc = glGetUniformLocation(_blur_shader, "source_texture");
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _glow_texture);
    if (sourceTexLoc >= 0) glUniform1i(sourceTexLoc, 0);
    if (directionLoc >= 0) glUniform2f(directionLoc, 1.0f, 0.0f);
    if (radiusLoc >= 0) glUniform1f(radiusLoc, 2.0f);
    
    renderFullscreenQuad();
    
    // 3. Apply vertical blur
    glBindFramebuffer(GL_FRAMEBUFFER, _glow_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindTexture(GL_TEXTURE_2D, _blur_texture);
    if (directionLoc >= 0) glUniform2f(directionLoc, 0.0f, 1.0f);
    
    renderFullscreenQuad();
    
    // 4. Final composition to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    // --- Use standard GL functions for composition shader --- 
    glUseProgram(_composite_shader); 
    // Declare uniform location variables (glowTexLoc was missing)
    GLint glowTexLoc = glGetUniformLocation(_composite_shader, "glow_texture");
    // Reuse existing sceneTexLoc and intensityLoc declared earlier in the function
    sceneTexLoc = glGetUniformLocation(_composite_shader, "scene_texture");
    intensityLoc = glGetUniformLocation(_composite_shader, "atmosphere_intensity");

    glActiveTexture(GL_TEXTURE0); // Bind scene texture to texture unit 0
    glBindTexture(GL_TEXTURE_2D, _scene_texture);
    if (sceneTexLoc >= 0) glUniform1i(sceneTexLoc, 0);

    glActiveTexture(GL_TEXTURE1); // Bind glow texture to texture unit 1
    glBindTexture(GL_TEXTURE_2D, _glow_texture); 
    if (glowTexLoc >= 0) glUniform1i(glowTexLoc, 1);
    
    if (intensityLoc >= 0) glUniform1f(intensityLoc, atmosphere_intensity); 

    renderFullscreenQuad();
}

void WebGLRenderer::renderFullscreenQuad() {
    // Create a fullscreen quad for rendering textures to the screen
    static const float quad_vertices[] = {
        // positions        // texture coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f
    };
    
    // Create temporary VAO and VBO for the quad
    GLuint quad_vao, quad_vbo;
    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);
    
    // Setup quad geometry
    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Draw the quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // Clean up
    glDeleteVertexArrays(1, &quad_vao);
    glDeleteBuffers(1, &quad_vbo);
}

void WebGLRenderer::cleanup() {
    if (_initialized) {
        cleanupFramebuffers();
        
        // Delete shader programs
        if (_blur_shader != 0) {
            glDeleteProgram(_blur_shader);
            _blur_shader = 0;
        }
        if (_composite_shader != 0) {
            glDeleteProgram(_composite_shader);
            _composite_shader = 0;
        }
        
        if (_context > 0) {
            emscripten_webgl_destroy_context(_context);
            _context = 0;
        }
        
        _initialized = false;
    }
}

} // namespace PixelTheater

#endif // defined(PLATFORM_WEB) || defined(EMSCRIPTEN) 