#include "PixelTheater/platform/webgl/shaders.h"

namespace PixelTheater {

// Main vertex shader for rendering LEDs
const char* vertex_shader_source = R"(#version 300 es
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform float led_size;
uniform float camera_distance;
uniform float canvas_height;

out vec3 fragColor;
out float point_size;
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
    float size_factor = canvas_height / 800.0;
    float base_size = 6.0 * led_size;
    float distance_scale = sqrt(camera_distance / max(viewDistance, 0.1));
    distance_scale = min(distance_scale, 2.0);
    
    // Reduce size of LEDs facing away from camera
    float angle_scale = mix(0.6, 1.0, smoothstep(-0.2, 0.5, viewAngle));
    
    gl_PointSize = max(base_size * size_factor * distance_scale * angle_scale, 1.5);
    point_size = gl_PointSize;
}
)";

// Main fragment shader for rendering LEDs
const char* fragment_shader_source = R"(#version 300 es
precision highp float;

in vec3 fragColor;
in float point_size;
in float viewDistance;
in vec3 viewNormal;
in float viewAngle;
uniform float atmosphere_intensity;

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
    // Calculate distance from center of point with enhanced anti-aliasing
    vec2 cxy = 2.0 * gl_PointCoord - 1.0;
    float r = dot(cxy, cxy);
    
    // Improved soft circle with better anti-aliasing
    float aa_width = 2.0 / point_size;
    float alpha = 1.0 - smoothstep(0.75 - aa_width, 0.75 + aa_width, r);
    
    // Enhanced attenuation for large points
    float attenuation = pow(1.0 - min(r, 1.0), 1.1);
    
    // Size-based brightness with atmospheric influence
    float size_ratio = point_size / 20.0; // Normalized size
    
    // Adjust size factor based on atmospheric intensity
    float atmos_factor = mix(1.0, 0.7, atmosphere_intensity / 3.0);
    float size_threshold = mix(1.5, 2.0, atmosphere_intensity / 3.0);
    
    // Less aggressive inverse size relationship
    float inverse_size_factor = 1.0 / max(size_ratio * atmos_factor, 0.3);
    float size_factor = clamp(inverse_size_factor, 0.5, 2.2); // Increased minimum brightness
    
    // Smoother brightness transition for larger sizes
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
    
    // Final color with enhanced scaling and size-based intensity
    vec3 color = fragColor;
    color.g *= 0.92; // Pre-adjust green channel
    
    // Improved size-dependent intensity scaling with atmospheric influence
    float intensity_scale = mix(2.2, 1.4, smoothstep(0.3, size_threshold, size_ratio));
    intensity_scale = mix(intensity_scale, intensity_scale * 1.2, atmosphere_intensity / 3.0);
    
    color = color * brightness * intensity_scale * mix(0.8, 1.2, dist_factor);
    
    // Apply view-dependent effects
    color *= back_dim;
    color *= mix(0.9, 1.1, scatter_boost);
    
    // Apply tone mapping before alpha
    color = toneMap(color);
    
    // Enhanced alpha for atmospheric effect with size consideration
    alpha *= mix(0.4, 1.0, view_fade);
    alpha *= mix(0.8, 1.0, scatter_boost);
    alpha *= mix(1.0, 0.85, smoothstep(0.3, size_threshold, size_ratio)); // Softer alpha reduction
    
    outColor = vec4(color, alpha);
    
    // Discard back-facing pixels with smoother transition
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

vec3 extractBrightAreas(vec3 color) {
    float brightness = perceivedBrightness(color);
    
    // Dynamic thresholds with adjusted green sensitivity
    float base_threshold = mix(0.25, 0.4, brightness);
    float softness = mix(0.3, 0.6, brightness);
    
    vec3 colorThresholds = vec3(
        base_threshold * 0.65,  // More sensitive to red
        base_threshold * 0.85,  // Less sensitive to green
        base_threshold * 0.6    // Most sensitive to blue
    );
    
    vec3 contribution;
    contribution.r = smoothstep(colorThresholds.r, colorThresholds.r + softness, color.r);
    contribution.g = smoothstep(colorThresholds.g, colorThresholds.g + softness, color.g);
    contribution.b = smoothstep(colorThresholds.b, colorThresholds.b + softness, color.b);
    
    // Enhanced dim boost with color preservation and green control
    float dimBoost = (1.0 - brightness) * 0.35;
    contribution = mix(contribution, vec3(1.0), dimBoost);
    contribution.g *= 0.9; // Additional green control
    
    vec3 result = color * contribution * 1.4;
    return toneMapGlow(result);
}

void main() {
    vec4 color = texture(scene_texture, TexCoords);
    vec3 brightAreas = extractBrightAreas(color.rgb);
    
    // Enhanced intensity scaling with better overflow handling
    float intensity_scale = atmosphere_intensity * 1.8;
    float boost = 1.0 + (1.0 - perceivedBrightness(color.rgb)) * 0.4;
    
    // Apply tone mapping to handle high intensities
    vec3 final = brightAreas * intensity_scale * boost;
    final = final / (1.0 + length(final) * 0.3);
    
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
    
    // Extended Gaussian weights for wider, softer blur
    float weights[15] = float[](
        0.0813, 0.0798, 0.0745, 0.0676, 0.0595,
        0.0508, 0.0420, 0.0336, 0.0261, 0.0196,
        0.0143, 0.0101, 0.0069, 0.0046, 0.0029
    );
    
    // Enhanced adaptive radius for more spread
    float radiusScale = 1.0 + length(blur_direction) * 0.6;
    float effectiveRadius = blur_radius * radiusScale * 2.0; // Increased radius for more spread
    
    // Center sample with reduced weight for more spread
    vec3 centerColor = texture(source_texture, TexCoords).rgb;
    result += centerColor * weights[0] * 0.9; // Reduced center weight
    total_weight += weights[0] * 0.9;
    
    // Enhanced sampling with better distribution and more spread
    for(int i = 1; i < 15; ++i) {
        float scale = float(i) / 14.0;
        vec2 offset = blur_direction * texelSize * (float(i) * effectiveRadius);
        
        // Wider sampling range
        vec2 samplePos1 = TexCoords + offset * scale * 1.2;
        vec2 samplePos2 = TexCoords - offset * scale * 1.2;
        
        vec3 color1 = texture(source_texture, samplePos1).rgb;
        vec3 color2 = texture(source_texture, samplePos2).rgb;
        
        // Enhanced weight calculation for softer falloff
        float w = weights[i] * (1.0 - scale * 0.1);
        result += (color1 + color2) * w;
        total_weight += 2.0 * w;
    }
    
    // Softer color preservation
    vec3 blurred = result / total_weight;
    blurred = pow(blurred, vec3(0.98)); // Softer gamma
    
    // Extended range handling with softer falloff
    blurred = blurred / (1.0 + 0.08 * length(blurred));
    
    FragColor = vec4(blurred, 1.0);
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
uniform mat4 model;  // Add model matrix uniform

out vec3 Normal;
out vec3 FragPos;

void main() {
    // Transform vertex position with model matrix first
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    
    // Transform normal with model matrix (ignoring translation)
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    Normal = normalMatrix * aNormal;
    
    // Final position in clip space
    gl_Position = projection * view * worldPos;
}
)";

// Mesh fragment shader
const char* mesh_fragment_shader_source = R"(#version 300 es
precision highp float;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 mesh_color;
uniform float opacity;
uniform vec3 light_position;

out vec4 FragColor;

void main() {
    // Base darker color for mesh
    vec3 color = mesh_color * 0.4;
    
    // Ambient component
    float ambient = 0.5;
    
    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light_position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Combine lighting
    vec3 result = (ambient + diff) * color;
    
    // Apply opacity
    FragColor = vec4(result, opacity);
}
)";

} // namespace PixelTheater 