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

out vec4 outColor;

void main() {
    // Calculate distance from center of point with anti-aliasing
    vec2 cxy = 2.0 * gl_PointCoord - 1.0;
    float r = dot(cxy, cxy);
    
    // Enhanced soft circle with better anti-aliasing and larger glow radius
    float aa_width = 2.0 / point_size;
    float alpha = 1.0 - smoothstep(0.75 - aa_width, 0.75 + aa_width, r);
    
    // Improved attenuation for large points with extended range
    float attenuation = pow(1.0 - min(r, 1.0), 1.1);
    float size_factor = clamp(point_size / 30.0, 0.2, 1.5);
    float brightness = mix(0.4, 1.2, size_factor) * attenuation;
    
    // Enhanced distance-based intensity with wider range
    float dist_factor = 1.0 - smoothstep(6.0, 40.0, viewDistance);
    
    // View-dependent darkening
    float view_fade = smoothstep(-0.3, 0.5, viewAngle); // Stronger fade for back-facing LEDs
    float back_dim = mix(0.1, 1.0, view_fade);          // Darken back-facing LEDs more
    
    // Apply depth-based atmospheric scattering
    float depth_scatter = 1.0 - smoothstep(10.0, 30.0, viewDistance);
    float scatter_boost = mix(0.2, 1.0, depth_scatter);
    
    // Final color with increased intensity and better scaling
    vec3 color = fragColor * brightness * 3.0 * mix(0.7, 1.2, dist_factor);
    
    // Apply view-dependent effects
    color *= back_dim;                    // Darken back-facing LEDs
    color *= mix(0.8, 1.0, scatter_boost); // Add slight atmospheric scattering
    
    // Fade out alpha for back-facing LEDs
    alpha *= mix(0.3, 1.0, view_fade);
    
    // Improved HDR handling with extended range
    color = color / (1.0 + 0.1 * length(color));
    
    outColor = vec4(color, alpha);
    
    // Discard back-facing pixels more aggressively
    if (r > 1.2 + aa_width || viewAngle < -0.5) discard;
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

// Enhanced brightness perception with better color handling
float perceivedBrightness(vec3 color) {
    // Adjusted weights for stronger glow on all colors
    return dot(color, vec3(0.25, 0.62, 0.13));
}

// Enhanced color-aware threshold function
vec3 extractBrightAreas(vec3 color) {
    float brightness = perceivedBrightness(color);
    
    // Lower thresholds for stronger effect
    float base_threshold = mix(0.2, 0.4, brightness);
    float softness = mix(0.6, 1.0, brightness);
    
    // Enhanced color-specific thresholds
    vec3 colorThresholds = vec3(
        base_threshold * 0.6,  // Even more sensitive to red
        base_threshold * 0.8,  // More sensitive to green
        base_threshold * 0.7   // More sensitive to blue
    );
    
    // Enhanced contribution calculation
    vec3 contribution;
    contribution.r = smoothstep(colorThresholds.r, colorThresholds.r + softness, color.r);
    contribution.g = smoothstep(colorThresholds.g, colorThresholds.g + softness, color.g);
    contribution.b = smoothstep(colorThresholds.b, colorThresholds.b + softness, color.b);
    
    // Stronger dim boost with better preservation
    float dimBoost = (1.0 - brightness) * 0.35;
    contribution = mix(contribution, vec3(1.0), dimBoost);
    
    // Enhanced color preservation with extended range
    vec3 result = color * contribution * 1.5; // Increased base intensity
    return result / (1.0 + 0.05 * length(result));
}

void main() {
    vec4 color = texture(scene_texture, TexCoords);
    vec3 brightAreas = extractBrightAreas(color.rgb);
    
    // Enhanced intensity scaling with extended range
    float intensity_scale = clamp(atmosphere_intensity * 1.2, 0.0, 4.0); // Increased range
    float boost = 1.0 + (1.0 - perceivedBrightness(color.rgb)) * 0.5;
    
    FragColor = vec4(brightAreas * intensity_scale * boost, 1.0);
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
    
    // Extended Gaussian weights for wider blur
    float weights[15] = float[](
        0.0913, 0.0838, 0.0745, 0.0636, 0.0525,
        0.0418, 0.0320, 0.0236, 0.0168, 0.0115,
        0.0076, 0.0048, 0.0029, 0.0017, 0.0009
    );
    
    // Enhanced adaptive radius
    float radiusScale = 1.0 + length(blur_direction) * 0.4;
    float effectiveRadius = blur_radius * radiusScale * 1.5; // Increased base radius
    
    // Center sample with enhanced weight
    vec3 centerColor = texture(source_texture, TexCoords).rgb;
    result += centerColor * weights[0];
    total_weight += weights[0];
    
    // Enhanced sampling with better distribution
    for(int i = 1; i < 15; ++i) {
        float scale = float(i) / 14.0;
        vec2 offset = blur_direction * texelSize * (float(i) * effectiveRadius);
        
        // Improved sampling with extended range
        vec2 samplePos1 = TexCoords + offset * scale;
        vec2 samplePos2 = TexCoords - offset * scale;
        
        vec3 color1 = texture(source_texture, samplePos1).rgb;
        vec3 color2 = texture(source_texture, samplePos2).rgb;
        
        // Enhanced weight calculation
        float w = weights[i] * (1.0 - scale * 0.15);
        result += (color1 + color2) * w;
        total_weight += 2.0 * w;
    }
    
    // Enhanced color preservation
    vec3 blurred = result / total_weight;
    blurred = pow(blurred, vec3(0.95)); // Adjusted gamma
    
    // Extended range handling
    blurred = blurred / (1.0 + 0.05 * length(blurred));
    
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

void main() {
    vec4 scene_color = texture(scene_texture, TexCoords);
    vec4 glow_color = texture(glow_texture, TexCoords);
    
    // Enhanced glow combination with extended range
    float intensity_scale = clamp(atmosphere_intensity * 1.0, 0.0, 4.0);
    vec3 glow = glow_color.rgb * intensity_scale * 1.3; // Increased base intensity
    
    // Enhanced atmosphere colors
    vec3 atmosphere_near = vec3(0.12, 0.18, 0.35); // Stronger near atmosphere
    vec3 atmosphere_far = vec3(0.18, 0.28, 0.45);  // Stronger far atmosphere
    
    float depth = length(glow);
    float depth_factor = smoothstep(0.0, 1.0, depth);
    vec3 atmosphere_tint = mix(atmosphere_near, atmosphere_far, depth_factor);
    
    // Enhanced color combination
    vec3 base_color = scene_color.rgb;
    vec3 glow_contribution = glow * (1.0 - depth_factor * 0.25); // Reduced depth attenuation
    vec3 atmosphere_contribution = atmosphere_tint * intensity_scale * 0.2; // Increased atmosphere
    
    // Improved combination with extended range
    vec3 final_color = base_color + glow_contribution;
    final_color += atmosphere_contribution * (1.0 - length(base_color) * 0.25);
    
    // Enhanced HDR handling with extended range
    final_color = final_color / (1.0 + length(final_color) * 0.15);
    
    // Ensure valid range while preserving intensity
    final_color = clamp(final_color, 0.0, 2.0); // Extended range
    
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