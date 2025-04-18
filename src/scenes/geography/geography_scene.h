#pragma once

#include "PixelTheater.h"
#include <cmath>
#include <string>
#include "PixelTheater/palettes.h"   // Include for Palettes
#include "PixelTheater/color_api.h"  // Include for colorFromPalette
#include "PixelTheater/core/math.h" // Include core math definitions (likely includes Vector3d)

// Use shorter Eigen types like in boids_scene.h
using Vector3f = Eigen::Vector3f;

// Use shorter names for constants if desired
using PixelTheater::Constants::PT_PI;
using PixelTheater::Constants::PT_TWO_PI;

namespace Scenes {

class GeographyScene : public PixelTheater::Scene {
public:
    GeographyScene() = default;

    void setup() override;
    void tick() override;
    virtual std::string status() const;

private:
    // Lorenz attractor parameters
    float sigma = 10.0f;
    float rho = 28.0f;
    float beta = 8.0f / 3.0f;
    float dt = 0.01f; // Time step for Lorenz calculation

    // Lorenz state variables
    float lorenz_x = 0.1f;
    float lorenz_y = 0.0f;
    float lorenz_z = 0.0f;

    // Scene animation state
    // Separate spin angles for each axis/gradient
    float spin_x = 0.0f;
    float spin_y = 0.0f;
    float spin_z = 0.0f;

    float model_radius = 150.0f; // Estimate or calculate in setup

    // Store estimated sphere radius if needed (Geography seems to use it indirectly)
    // float sphere_radius = 1.0f; // Default or estimate in setup

    // Helper for Lorenz calculation
    void updateLorenz();
};

} // namespace Scenes 

// No implementations in header anymore 