#pragma once

#include "PixelTheater/SceneKit.h"
#include <vector>

// Note: Eigen is included via SceneKit.h -> math.h

namespace Scenes {

class GravityMarblesScene : public Scene {
public:
    GravityMarblesScene() = default;
    ~GravityMarblesScene() override = default;

    void setup() override;
    void tick() override;
    std::string status() const override;

private:
    struct Marble {
        Eigen::Vector3f position;
        Eigen::Vector3f velocity;
        float radius;
        CRGB color;
        uint32_t id;
    };

    std::vector<Marble> marbles_;
    Eigen::Vector3f gravity_ = {0.0f, 0.0f, -1.0f}; // Default gravity direction
    Eigen::Vector3f lin_accel_ = {0.0f, 0.0f, 0.0f}; // Linear acceleration from device movement
    uint32_t next_id_ = 0;

    // Parameters cache
    int population_;
    float marble_size_;
    float gravity_strength_;
    float damping_;
    float elasticity_;
    uint8_t trail_amount_;
    float inertia_;

    // Helper methods
    void initializeMarbles();
    void updatePhysics(float dt);
    void handleCollisions();
    void renderMarbles();
    void renderGravityIndicator();
};

} // namespace Scenes 