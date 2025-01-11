#include "animation.h"
#include "animations/wandering_particles.h"

void WanderingParticles::init(const AnimParams& params) {
    Animation::init(params);
    
    // Initialize particles
    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i] = new Particle();
        reset_particle(particles[i]);
    }
}

void WanderingParticles::tick() {
    fadeToBlackBy(leds, numLeds(), 20);
    
    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i]->tick();
        
        int led_num = particles[i]->led_number;
        if (led_num >= 0 && led_num < numLeds()) {
            nblend(leds[led_num], particles[i]->color, 80/(particles[i]->hold_time - particles[i]->age + 1));
        }
        
        if (random8() < 2) {
            reset_particle(particles[i]);
        }
    }
}

void WanderingParticles::reset_particle(Particle* p) {
    p->reset();
    // Random starting position around the sphere
    p->led_number = random(numLeds());
    uint8_t lev = random(10,50);
    p->color = (uint32_t)CRGB(lev, random(100,230), lev);
    
    // Random starting angles
    p->a = random(TWO_PI*1000)/1000.0;  // azimuth angle (0 to 2π)
    p->c = random(PI*1000)/1000.0;      // polar angle (0 to π)
    
    // Random velocities in both directions
    p->av = random(-10,10)/1000.0;    // azimuthal velocity
    p->cv = random(-10,10)/1000.0;    // polar velocity
}

String WanderingParticles::getStatus() const {
    output.printf("Particles: %d", NUM_PARTICLES);
    return output.get();
} 