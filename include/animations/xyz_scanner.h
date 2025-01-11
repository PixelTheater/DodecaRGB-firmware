#pragma once
#include "animation.h"

class XYZScanner : public Animation {
private:
    float max_range = 450;
    float zi = -max_range;
    float yi = -max_range;
    float xi = -max_range;
    float target = 140;
    int counter = 0;
    int min_off = 0;
    float speed = 1.0;
    int blend = 160;
    uint8_t fade_amount = 35;

public:
    const char* getName() const override { return "xyz_scanner"; }
    
    AnimParams getDefaultParams() const override {
        AnimParams p;
        p.setFloat("speed", 0.05);
        p.setInt("blend", 160);
        p.setInt("fade", 35);
        return p;
    }

    void init(const AnimParams& params) override;
    void tick() override;

    String getStatus() const override {
        output.printf("XYZ Scanner: counter=%d\n", counter);
        output.printf("Positions: x=%.1f y=%.1f z=%.1f\n", xi, yi, zi);
        output.printf("Target: %.1f Speed: %.3f Blend: %d Fade: %d\n", 
            target, speed, blend, fade_amount);
        return output.get();
    }
    
    AnimParams getPreset(const String& preset_name) const override {
        if (preset_name == "fast") {
            AnimParams p;
            p.setFloat("speed", 3.5);    // 4x faster
            p.setInt("blend", 100);       // More aggressive blending
            p.setInt("fade", 10);         // longer trails
            return p;
        }
        if (preset_name == "slow") {
            AnimParams p;
            p.setFloat("speed", 1.0);   // Slower movement
            p.setInt("blend", 20);       // Gentler blending
            p.setInt("fade", 30);         // Faster fade
            return p;
        }
        return getDefaultParams();
    }
}; 