#pragma once
#include "animation.h"
#include "palettes.h"
#include <vector>

class ColorShow : public Animation {
private:
    int show_pos = 0;
    int show_color = 0;
    static const int led_limit = 100;
    struct Segment {
        uint8_t start_led;
        uint8_t len;
        float pos;
        float speed;
        CRGB center_color;
        int lifetime;
    };
    std::vector<Segment> segments;
    Segment createRandomSegment();

public:
    void init(const AnimParams& params) override;
    void tick() override;
    String getStatus() const override;
    const char* getName() const override { return "colorshow"; }
}; 