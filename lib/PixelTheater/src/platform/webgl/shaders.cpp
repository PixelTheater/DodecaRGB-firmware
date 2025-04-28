#include "PixelTheater/platform/webgl/shaders.h"

namespace PixelTheater {

// Main vertex shader for rendering LEDs (Points)
const char* vertex_shader_source = R"(#version 300 es
layout(location = 0) in vec3 position; // Note: Back to 'position' at location 0
layout(location = 1) in vec3 color;    // Note: Back to 'color' at location 1

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform float led_size;
uniform float camera_distance;
uniform float canvas_height;

out vec3 fragColor;
out float point_size; // Output point size for fragment shader
out float viewDistance;
out vec3 viewNormal;    // Normal vector in view space
out float viewAngle;    // Angle between view direction and normal

void main() {
    // Transform the vertex
    vec4 worldPos = model * vec4(position, 1.0);
    vec4 viewPos = view * worldPos;
    gl_Position = projection * viewPos;
    
    // Calculate view distance for atmospheric effects
    viewDistance = length(viewPos.xyz);
    
    // Calculate normal in view space (assuming point is on sphere surface)
    viewNormal = normalize(mat3(view * model) * normalize(position));
    
    // Calculate angle between view direction and normal
    vec3 viewDir = normalize(-viewPos.xyz);
    viewAngle = dot(viewNormal, viewDir);
    
    // Pass color to fragment shader
    fragColor = color;
    
    // Calculate point size based on distance and view angle
    float size_factor = canvas_height / 800.0; // Normalize based on default height
    float base_size = 6.0 * led_size; // Base size, adjust 6.0 as needed
    float distance_scale = sqrt(camera_distance / max(viewDistance, 0.1));
    distance_scale = min(distance_scale, 2.0); // Clamp max scaling
    
    // Reduce size of LEDs facing away from camera
    float angle_scale = mix(0.6, 1.0, smoothstep(-0.2, 0.5, viewAngle));
    
    gl_PointSize = max(base_size * size_factor * distance_scale * angle_scale, 1.5);
    point_size = gl_PointSize; // Pass calculated size to fragment shader
}
)";

// Main fragment shader for rendering LEDs (Points)
const char* fragment_shader_source = R"(#version 300 es
precision highp float;

in vec3 fragColor;
in float point_size; // Receive point size from vertex shader
in float viewDistance;
in vec3 viewNormal;
in float viewAngle;
uniform float atmosphere_intensity;
// REMOVED: uniform float led_size;

out vec4 outColor;

// Robust HDR tone mapping with enhanced green channel handling
vec3 toneMap(vec3 color) {
    // Adjust green channel sensitivity
    vec3 weights = vec3(0.2126, 0.9152, 0.0722);
    float luminance = dot(color, weights);
    
    // Separate handling for green channel
    float green_intensity = color.g * 0.85;
    
    // Extended dynamic range tone mapping with green adjustment
    float mapped = luminance / (1.0 + luminance);
    mapped = pow(mapped, 0.95);
    
    // Color normalization with green channel compensation
    vec3 normalized = color;
    normalized.g = green_intensity;
    normalized = normalized / max(luminance, 0.001);
    
    // Apply final scaling with controlled green boost
    vec3 final = normalized * mapped * 1.2;
    final.g *= 0.9;
    
    return final;
}

void main() {
    // Calculate distance from center of point using gl_PointCoord
    vec2 cxy = 2.0 * gl_PointCoord - 1.0;
    float r = dot(cxy, cxy);
    
    // Calculate alpha for soft circle based on distance and point size
    float aa_width = 2.0 / point_size; // Anti-aliasing width based on point size
    float alpha = 1.0 - smoothstep(0.75 - aa_width, 0.75 + aa_width, r);
    
    // Attenuation towards edge of point
    float attenuation = pow(1.0 - min(r, 1.0), 1.1);
    
    // Size-based brightness calculations (restored)
    float size_ratio = point_size / 20.0; // Normalized size (adjust 20.0 if needed)
    float atmos_factor = mix(1.0, 0.7, atmosphere_intensity / 3.0);
    float size_threshold = mix(1.5, 2.0, atmosphere_intensity / 3.0);
    float inverse_size_factor = 1.0 / max(size_ratio * atmos_factor, 0.3);
    float size_factor = clamp(inverse_size_factor, 0.5, 2.2);
    float base_brightness = mix(1.6, 0.8, smoothstep(0.3, size_threshold, size_ratio));
    float brightness = base_brightness * attenuation * size_factor;
    
    // Enhanced distance-based intensity with atmospheric influence
    float dist_factor = 1.0 - smoothstep(8.0, 35.0, viewDistance);
    
    // View-dependent darkening with smoother transition
    float view_fade = smoothstep(-0.2, 0.6, viewAngle);
    float back_dim = mix(0.15, 1.0, view_fade);
    
    // Enhanced depth-based atmospheric scattering
    float depth_scatter = 1.0 - smoothstep(12.0, 25.0, viewDistance);
    float scatter_boost = mix(0.3, 1.2, depth_scatter);
    
    // Final color calculation (restored size dependency)
    vec3 color = fragColor;
    color.g *= 0.92; // Pre-adjust green channel
    float intensity_scale = mix(2.2, 1.4, smoothstep(0.3, size_threshold, size_ratio));
    intensity_scale = mix(intensity_scale, intensity_scale * 1.2, atmosphere_intensity / 3.0);
    
    color = color * brightness * intensity_scale * mix(0.8, 1.2, dist_factor);
    
    // Apply view-dependent effects
    color *= back_dim;
    color *= mix(0.9, 1.1, scatter_boost);
    
    // Apply tone mapping before alpha
    color = toneMap(color);
    
    // Final alpha calculation (restored size dependency)
    alpha *= mix(0.4, 1.0, view_fade);
    alpha *= mix(0.8, 1.0, scatter_boost);
    alpha *= mix(1.0, 0.85, smoothstep(0.3, size_threshold, size_ratio));
    
    outColor = vec4(color, alpha);
    
    // Discard fragments outside the circle and those facing away
    if (r > 1.1 + aa_width || viewAngle < -0.6) discard;
}
)";

// Quad vertex shader (used for post-processing)
const char* quad_vertex_shader_source = R"(#version 300 es
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoords;

void main() {
    gl_Position = vec4(aPos, 1.0);
    TexCoords = aTexCoord;
}
)";

// Glow vertex shader (reuses quad vertex shader)
const char* glow_vertex_shader_source = R"(#version 300 es
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoords;

void main() {
    gl_Position = vec4(aPos, 1.0);
    TexCoords = aTexCoord;
}
)";

// Glow fragment shader (extracts bright areas)
const char* glow_fragment_shader_source = R"(#version 300 es
precision highp float;

uniform sampler2D scene_texture;
uniform float atmosphere_intensity;

in vec2 TexCoords;
out vec4 FragColor;

// Enhanced brightness perception with better green handling
float perceivedBrightness(vec3 color) {
    return dot(color, vec3(0.22, 0.55, 0.13)); // Reduced green weight
}

// Improved tone mapping for glow that preserves color with green control
vec3 toneMapGlow(vec3 color) {
    vec3 weights = vec3(0.2126, 0.7152, 0.0722);
    float luma = dot(color, weights);
    
    // Separate green handling
    color.g *= 0.9; // Reduce green intensity in glow
    
    float toneMapped = luma / (1.0 + luma * 0.5);
    return color * (toneMapped / max(luma, 0.001)) * 1.3;
}

void main() {
    vec4 color = texture(scene_texture, TexCoords);
    
    // --- Simplified Glow Extraction (Ensure Active) ---
    float brightness = perceivedBrightness(color.rgb);
    float threshold = 0.1; // Keep low threshold for now
    vec3 brightAreas = (brightness > threshold) ? color.rgb : vec3(0.0); // Pass raw color if bright
    // --- End Simplified --- 

    // Apply tone mapping *after* extraction to handle HDR input
    vec3 final = toneMapGlow(brightAreas); // Use the existing toneMapGlow
    
    FragColor = vec4(final, 1.0); 
}
)";

// Blur shader (used for horizontal and vertical passes)
const char* blur_vertex_shader_source = R"(#version 300 es
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoords;

void main() {
    gl_Position = vec4(aPos, 1.0);
    TexCoords = aTexCoord;
}
)";

// Gaussian blur fragment shader
const char* blur_fragment_shader_source = R"(#version 300 es
precision highp float;

uniform sampler2D source_texture;
uniform vec2 blur_direction;
uniform float blur_radius;

in vec2 TexCoords;
out vec4 FragColor;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(source_texture, 0));
    vec3 result = vec3(0.0);
    float total_weight = 0.0;

    // Simplified Gaussian weights (5 samples)
    float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

    // Center sample
    result += texture(source_texture, TexCoords).rgb * weights[0];
    total_weight += weights[0];

    // Samples in blur direction
    for(int i = 1; i < 5; ++i) {
        vec2 offset = blur_direction * texelSize * float(i) * blur_radius;
        result += texture(source_texture, TexCoords + offset).rgb * weights[i];
        result += texture(source_texture, TexCoords - offset).rgb * weights[i];
        total_weight += 2.0 * weights[i];
    }

    FragColor = vec4(result / total_weight, 1.0);
}
)";

// Final composition shader
const char* composite_fragment_shader_source = R"(#version 300 es
precision highp float;

uniform sampler2D scene_texture;
uniform sampler2D glow_texture;
uniform float atmosphere_intensity;

in vec2 TexCoords;
out vec4 FragColor;

// Enhanced tone mapping that preserves atmosphere with softer blend
vec3 finalToneMap(vec3 color) {
    // Split color into base and highlight components
    vec3 weights = vec3(0.2126, 0.7152, 0.0722);
    float luminance = dot(color, weights);
    float highlight = max(luminance - 1.0, 0.0);
    
    // Adjust green channel in highlights
    color.g *= mix(1.0, 0.9, smoothstep(0.8, 1.2, color.g));
    
    // Map base and highlights separately with softer transition
    vec3 base = color / (1.0 + luminance * 0.6); // Increased denominator for softer effect
    vec3 highlights = color * (highlight / (1.0 + highlight * 1.2)); // Softer highlight scaling
    
    // Blend results with softer transition
    vec3 result = mix(base, highlights, smoothstep(0.7, 1.3, luminance));
    result.g *= 0.95;
    
    return result;
}

void main() {
    vec4 scene_color = texture(scene_texture, TexCoords);
    vec4 glow_color = texture(glow_texture, TexCoords);
    
    // Softer glow combination
    float intensity_scale = atmosphere_intensity * 1.2; // Reduced from 1.5
    vec3 glow = glow_color.rgb * intensity_scale;
    glow.g *= 0.9;
    
    // Softer atmosphere colors
    vec3 atmosphere_near = vec3(0.10, 0.11, 0.25); // Reduced intensity
    vec3 atmosphere_far = vec3(0.12, 0.14, 0.35); // Reduced intensity
    
    float depth = length(glow);
    float depth_factor = smoothstep(0.0, 1.0, depth);
    
    // Enhanced fog-like effect
    vec3 atmosphere_tint = mix(atmosphere_near, atmosphere_far, depth_factor);
    float fog_factor = smoothstep(0.2, 0.8, depth_factor); // Smoother fog transition
    
    // Enhanced color combination with fog-like blend
    vec3 base_color = scene_color.rgb;
    vec3 glow_contribution = glow * (1.0 - depth_factor * 0.3);
    vec3 atmosphere_contribution = atmosphere_tint * intensity_scale * 0.2; // Reduced from 0.25
    
    // Combine with enhanced fog-like effect
    vec3 final_color = base_color;
    final_color = mix(final_color, glow_contribution, fog_factor * 0.6); // Softer glow blend
    final_color += atmosphere_contribution * (1.0 - length(base_color) * 0.25);
    
    // Additional fog blend
    vec3 fog_color = atmosphere_tint * 0.5;
    final_color = mix(final_color, fog_color, fog_factor * 0.15); // Subtle fog overlay
    
    // Apply final tone mapping with softer preservation
    final_color = finalToneMap(final_color);
    
    FragColor = vec4(final_color, 1.0);
}
)";

// Mesh vertex shader
const char* mesh_vertex_shader_source = R"(#version 300 es
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform float mesh_opacity;

out vec3 Normal; // Output Normal in View Space
out vec3 FragPos; // Output FragPos in View Space
out float Opacity;

void main() {
    // Calculate position and normal in view space
    vec4 viewPos = view * model * vec4(aPos, 1.0);
    FragPos = viewPos.xyz / viewPos.w; // Perspective divide for view space position
    
    // Normal needs to be transformed using the normal matrix derived from view*model
    mat3 normalMatrix = mat3(transpose(inverse(view * model)));
    Normal = normalize(normalMatrix * aNormal);
    
    // Pass opacity to fragment shader
    Opacity = mesh_opacity;
    
    // Final position in clip space (already calculated indirectly)
    gl_Position = projection * viewPos;
}
)";

// Mesh fragment shader
const char* mesh_fragment_shader_source = R"(#version 300 es
precision highp float;

in vec3 Normal; // Now in View Space
in vec3 FragPos; // Now in View Space
in float Opacity;

uniform vec3 mesh_color;
// REMOVED: uniform vec3 light_position; // Light is at camera (0,0,0) in view space
uniform bool is_wireframe; 

out vec4 FragColor;

void main() {
    if (is_wireframe) {
        // Wireframe: Use color and opacity uniforms directly
        FragColor = vec4(mesh_color, Opacity);
    } else {
        // Faces: Lighting in View Space
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(-FragPos); // Vector from fragment to camera (origin in view space)
        
        // Light direction from fragment to light (camera at origin)
        vec3 lightDir = normalize(-FragPos); // Same as viewDir when light is at camera origin
        
        // Ambient
        float ambient = 0.3; 
        
        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuseColor = diff * mesh_color;
        
        // Specular (Phong)
        float specularStrength = 0.25; // Reduced brightness of specular highlights
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0); // Adjust shininess (16.0 is moderate)
        vec3 specularColor = specularStrength * spec * vec3(1.0, 1.0, 1.0); // White highlights

        // Combine lighting
        vec3 litColor = (ambient * mesh_color) + diffuseColor + specularColor;
        
        // Apply opacity
        FragColor = vec4(litColor, Opacity);
    }
}
)";

} // namespace PixelTheater 