#pragma once
#include "animation.h"

class IdentifySides : public Animation {
private:
    float speed;
    uint32_t counter = 0;
    int current_face = 0;
    CRGBPalette16 palette;

public:
    void init(const AnimParams& params) override;
    void tick() override;
    String getStatus() const override;
    const char* getName() const override { return "identify_sides"; }
}; 