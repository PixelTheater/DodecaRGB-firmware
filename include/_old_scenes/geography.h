#pragma once

#include "animation.h"

class Geography : public Animation {
private:
    float spin_angle = 16.0;
    float shift = 0;
    float spin_dir = 0.0;
    
    // Lorenz parameters
    float sigma = 8.0;
    float rho = 24.0;
    float beta = 8.0 / 3.0;
    float dt = 0.002;
    float x = 0.1, y = 0.3, z = -0.2;
    const int sphere_r = 310;
    
public:
    Geography() = default;
    void init(const AnimParams& params) override;
    void tick() override;
    String getStatus() const override;
    const char* getName() const override { return "geography"; }
    AnimParams getPreset(const String& preset_name) const override {
        if (preset_name == "default") {
            AnimParams p;
            return p;
        }
        return getDefaultParams();
    }
}; 