#pragma once

#include "animation.h"
#include "palettes.h"

class Sparkles : public Animation {
private:
    // Parameters
    int period;
    CRGBPalette16 base_palette;
    CRGBPalette16 highlight_palette;
    
    // State variables
    uint16_t seed1, seed2;
    uint8_t color_mix;
    uint8_t power_fade = 20;
    int num_picks;
    uint8_t blend1, blend2;
    CRGB c, c2;

public:
    Sparkles() = default;
    void init(const AnimParams& params) override;
    void tick() override;
    String getStatus() const override;
    const char* getName() const override { return "sparkles"; }
    AnimParams getPreset(const String& preset_name) const override {
        if (preset_name == "default") {
            AnimParams p;
            p.setInt("period", 580);
            p.setPalette("base_palette", basePalette);
            p.setPalette("highlight_palette", highlightPalette);
            return p;
        }
        return getDefaultParams();  // Fallback to defaults if preset not found
    }
};
