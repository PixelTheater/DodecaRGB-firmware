#pragma once
#include "animation.h"

class XYZScanner : public Animation {
private:
    float max_range = 500;
    float zi = -max_range;
    float yi = -max_range;
    float xi = -max_range;
    float target = 140;
    int counter = 0;
    int min_off = 0;
    float speed = 0.005;
    int blend = 160;
    uint8_t fade_amount = 35;

public:
    const char* getName() const override { return "xyz_scanner"; }
    
    AnimParams getDefaultParams() const override {
        AnimParams p;
        p.setFloat("speed", 0.005);
        p.setInt("blend", 160);
        p.setInt("fade", 35);
        return p;
    }

    void init(const AnimParams& params) override;
    void tick() override;
}; 