#include "geography_scene.h"
#include <cmath>      // For sqrt, acos, atan2, cos, sin, fmod
#include <algorithm>  // For std::clamp
#include <cstdio>     // For snprintf

namespace Scenes {

using PixelTheater::Constants::PT_TWO_PI;

// Add the using alias here too for consistency within the file
using Vector3f = Eigen::Vector3f;
using Matrix3f = Eigen::Matrix3f; // Add alias for Matrix3f

// Removed rotatePoint helper

// --- Implementations ---

void GeographyScene::setup() {
    set_name("Geography");
    set_description("Lorenz attractor driving 3 rotating color gradients");
    set_version("1.1"); // Incremented version
    set_author("Original Author (Refactored)");

    // Define parameters
    param("sigma", "range", 5.0f, 20.0f, 10.0f, "clamp", "Lorenz sigma");
    param("rho", "range", 10.0f, 50.0f, 28.0f, "clamp", "Lorenz rho");
    param("beta", "range", 1.0f, 5.0f, 8.0f / 3.0f, "clamp", "Lorenz beta");
    param("dt", "range", 0.001f, 0.015f, 0.007f, "clamp", "Sim speed (smaller=slower)"); // Slower default dt
    param("spin_speed_x", "range", 0.0f, 0.5f, 0.027f, "clamp", "Gradient X spin rate");
    param("spin_speed_y", "range", 0.0f, 0.5f, 0.033f, "clamp", "Gradient Y spin rate");
    param("spin_speed_z", "range", 0.0f, 0.5f, 0.041f, "clamp", "Gradient Z spin rate");
    param("dimming", "range", 0.1f, 1.0f, 0.4f, "clamp", "Overall brightness scale");

    // Initialize state - Apply randomization like old init
    sigma = settings["sigma"];
    rho = settings["rho"];
    beta = settings["beta"];
    dt = settings["dt"];

    // Slight randomization
    sigma += randomFloat(-2.0f, 2.0f);
    rho += randomFloat(-2.0f, 2.0f);
    beta += randomFloat(-0.5f, 0.5f);

    // Clamp initial randomized values
    sigma = std::clamp(sigma, 5.0f, 20.0f);
    rho = std::clamp(rho, 10.0f, 50.0f);
    beta = std::clamp(beta, 1.0f, 5.0f);

    // Reset other state variables
    lorenz_x = randomFloat(-0.1f, 0.1f);
    lorenz_y = randomFloat(-0.1f, 0.1f);
    lorenz_z = randomFloat(-0.1f, 0.1f);
    spin_x = 0.0f;
    spin_y = 0.0f;
    spin_z = 0.0f;

    // Estimate model radius based on points
    float max_r_sq = 0.0f;
    for(size_t i=0; i < model().pointCount(); ++i) {
        const auto& pt = model().point(i);
        float r_sq = pt.x()*pt.x() + pt.y()*pt.y() + pt.z()*pt.z();
        if (r_sq > max_r_sq) max_r_sq = r_sq;
    }
    if (max_r_sq > 1e-6f) {
        model_radius = sqrt(max_r_sq);
        logInfo("Estimated model radius: %.2f", model_radius);
    } else {
        logWarning("Could not estimate model radius, using default: %.1f", model_radius);
    }
}

void GeographyScene::updateLorenz() {
    // Read current parameter values that affect simulation
    sigma = settings["sigma"];
    rho = settings["rho"];
    beta = settings["beta"];
    float sim_dt = settings["dt"]; // Use the dt parameter for sim speed

    // Calculate Lorenz system derivatives
    float dx = sigma * (lorenz_y - lorenz_x);
    float dy = lorenz_x * (rho - lorenz_z) - lorenz_y;
    float dz = lorenz_x * lorenz_y - beta * lorenz_z;

    // Update state variables using sim_dt
    lorenz_x += dx * sim_dt;
    lorenz_y += dy * sim_dt;
    lorenz_z += dz * sim_dt;
}

void GeographyScene::tick() {
    Scene::tick(); // Increment base tick counter

    updateLorenz();

    // Normalize Lorenz state for rotation control
    float norm_x = map(lorenz_x, -25.0f, 25.0f, -1.0f, 1.0f);
    float norm_y = map(lorenz_y, -35.0f, 35.0f, -1.0f, 1.0f);
    float norm_z = map(lorenz_z, 0.0f, 50.0f, -1.0f, 1.0f); // Z usually positive
    norm_x = std::clamp(norm_x, -1.0f, 1.0f);
    norm_y = std::clamp(norm_y, -1.0f, 1.0f);
    norm_z = std::clamp(norm_z, -1.0f, 1.0f);

    // Update spin angles based on Lorenz state and speed parameters
    float speed_x = settings["spin_speed_x"];
    float speed_y = settings["spin_speed_y"];
    float speed_z = settings["spin_speed_z"];

    spin_x += norm_x * speed_x; // Modulate speed by attractor state
    spin_y += norm_y * speed_y;
    spin_z += norm_z * speed_z;

    // Define base axes
    Vector3f axis_x(1.0f, 0.0f, 0.0f);
    Vector3f axis_y(0.0f, 1.0f, 0.0f);
    Vector3f axis_z(0.0f, 0.0f, 1.0f);

    // Manually construct rotation matrices using sin/cos (from <cmath>)
    float cosX = cosf(spin_x), sinX = sinf(spin_x);
    float cosY = cosf(spin_y), sinY = sinf(spin_y);
    float cosZ = cosf(spin_z), sinZ = sinf(spin_z);

    Eigen::Matrix3f rot_x_mat;
    rot_x_mat << 1,    0,     0,
                 0, cosX, -sinX,
                 0, sinX,  cosX;

    Eigen::Matrix3f rot_y_mat;
    rot_y_mat << cosY, 0, sinY,
                    0, 1,    0,
                -sinY, 0, cosY;

    Eigen::Matrix3f rot_z_mat;
    rot_z_mat << cosZ, -sinZ, 0,
                 sinZ,  cosZ, 0,
                    0,     0, 1;

    // Define the 3 axes for our gradients (can be simple or rotated)
    // Let's rotate the standard X, Y, Z axes by their respective spins
    Vector3f gradient_axis1 = rot_x_mat * axis_x; // X-axis rotated around X (stays X)
    Vector3f gradient_axis2 = rot_y_mat * axis_y; // Y-axis rotated around Y (stays Y)
    Vector3f gradient_axis3 = rot_z_mat * axis_z; // Z-axis rotated around Z (stays Z)
    // This initial setup isn't very dynamic. Let's make them interact more.
    // Maybe rotate axis1 by spin_x, axis2 by spin_y, axis3 by spin_z?
    // Let's try applying all spins to each axis for more complexity:
    // Matrix3f rotation = rot_z_mat * rot_y_mat * rot_x_mat;
    // Vector3f gradient_axis1 = rotation * axis_x;
    // Vector3f gradient_axis2 = rotation * axis_y;
    // Vector3f gradient_axis3 = rotation * axis_z;
    // Or maybe simpler: rotate X by spin_y, Y by spin_z, Z by spin_x?
    gradient_axis1 = rot_y_mat * axis_x;
    gradient_axis2 = rot_z_mat * axis_y;
    gradient_axis3 = rot_x_mat * axis_z;

    // Define palettes to use
    const auto& palette1 = PixelTheater::Palettes::RainbowColors;
    const auto& palette2 = PixelTheater::Palettes::OceanColors;
    const auto& palette3 = PixelTheater::Palettes::LavaColors;
    uint8_t dimming_factor = static_cast<uint8_t>((float)settings["dimming"] * 255.0f);

    for (size_t i = 0; i < ledCount(); i++) {
        const auto& p = model().point(i);
        Vector3f p_vec(p.x(), p.y(), p.z());

        // Project point onto the 3 rotating gradient axes
        float dot1 = p_vec.dot(gradient_axis1);
        float dot2 = p_vec.dot(gradient_axis2);
        float dot3 = p_vec.dot(gradient_axis3);

        // Map dot product [-radius, +radius] to palette index [0, 255]
        uint8_t index1 = static_cast<uint8_t>(map(dot1, -model_radius, model_radius, 0.0f, 255.0f));
        uint8_t index2 = static_cast<uint8_t>(map(dot2, -model_radius, model_radius, 0.0f, 255.0f));
        uint8_t index3 = static_cast<uint8_t>(map(dot3, -model_radius, model_radius, 0.0f, 255.0f));

        // Get colors from the palettes
        PixelTheater::CRGB color1 = PixelTheater::colorFromPalette(palette1, index1);
        PixelTheater::CRGB color2 = PixelTheater::colorFromPalette(palette2, index2);
        PixelTheater::CRGB color3 = PixelTheater::colorFromPalette(palette3, index3);

        // Blend the colors using nblend for less saturation
        PixelTheater::CRGB final_color = color1.fadeToBlackBy(128);
        PixelTheater::nblend(final_color, color2, 80); // Blend 50% of color2
        PixelTheater::nblend(final_color, color3, 80);  // Blend 33% of color3 (approx)

        // Apply dimming
        final_color.nscale8(dimming_factor);

        leds[i] = final_color;
    }
}

std::string GeographyScene::status() const {
    char buffer[128];
    // Update status to show Lorenz state and spin angles
    snprintf(buffer, sizeof(buffer), "Spin: %.1f,%.1f,%.1f | L: %.1f,%.1f,%.1f",
             spin_x, spin_y, spin_z, lorenz_x, lorenz_y, lorenz_z);
    return std::string(buffer);
}

} // namespace Scenes 