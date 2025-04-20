#pragma once

#include "PixelTheater/SceneKit.h"

#include <vector>
#include <cmath>
#include <string>

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
    virtual std::string status() const override;

    void update_attractor(float dt);
    void draw_lorenz();
    void draw_texture_mapped();

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

    // Helper for Lorenz calculation
    void updateLorenz();
};

} // namespace Scenes 

// No implementations in header anymore 