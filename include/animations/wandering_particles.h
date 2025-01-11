#pragma once

#include "animation.h"
#include "particle.h"

class WanderingParticles : public Animation {
private:
    static const int NUM_PARTICLES = 80;
    Particle* particles[NUM_PARTICLES];
    
    void reset_particle(Particle* p);
    
public:
    WanderingParticles() = default;
    void init(const AnimParams& params) override;
    void tick() override;
    String getStatus() const override;
    const char* getName() const override { return "wandering_particles"; }
    AnimParams getPreset(const String& preset_name) const override {
        if (preset_name == "default") {
            AnimParams p;
            // Add any default parameters here if needed
            return p;
        }
        return getDefaultParams();
    }
}; 