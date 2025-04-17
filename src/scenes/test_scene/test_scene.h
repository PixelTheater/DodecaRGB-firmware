#pragma once

#include "PixelTheater.h" // Main library include
#include "benchmark.h"
#include "models/DodecaRGBv2/model.h" // Correct path to model header
#include <cmath> // For sin/cos
#include <vector> // For storing face LEDs if needed

// External declaration of debug mode flag
extern bool g_debug_mode;

namespace Scenes {

// Define the concrete model type used by this scene
using ModelDef = PixelTheater::Models::DodecaRGBv2; // Correct model struct name

// Test Scene Implementation
// Enhanced to provide more visual diagnostics
class TestScene : public PixelTheater::Scene {
private:
    enum TestPhase {
        FADE_IN,
        FACE_CYCLE,
        LED_CHASE,
        BRIGHTNESS_PULSE,
        RAINBOW_CYCLE,
        NUM_PHASES // Keep last
    };

    TestPhase current_phase_ = FADE_IN;
    float phase_timer_ = 0.0f; // Time elapsed within the current phase
    int current_face_ = 0;
    int chase_position_ = 0;
    uint8_t base_hue_ = 0;
    float pulse_timer_ = 0.0f; 
    bool is_pulsing_ = false;

    // Phase durations (approximate seconds)
    const float DURATION_FADE_IN = 2.0f;
    const float DURATION_FACE_CYCLE = 30.0f;
    const float DURATION_LED_CHASE = 30.0f;
    const float DURATION_BRIGHTNESS_PULSE = 30.0f;
    const float DURATION_RAINBOW_CYCLE = 30.0f;

    const uint8_t MAX_BRIGHTNESS = 100; // General brightness limit (0-255)
    const uint8_t PULSE_BRIGHTNESS = 255; // Full brightness for pulses
    const float PULSE_INTERVAL = 3.0f; // Seconds between pulses
    const float PULSE_DURATION = 0.15f; // Seconds pulse stays at max bright

public:
    TestScene() : PixelTheater::Scene() {} 

    void setup() override {
        set_name("Diagnostic Test"); 
        set_description("Cycles through visual tests for model/LEDs");
        set_version("1.1");
        set_author("PixelTheater Dev");

        // No parameters needed for this version
        logInfo("Diagnostic Test Scene Setup Complete");
        resetState();
    }

    void reset() override {
        Scene::reset(); // Call base class reset
        resetState();
        logInfo("Diagnostic Test Scene Reset");
    }

    void resetState() {
        current_phase_ = FADE_IN;
        phase_timer_ = 0.0f;
        current_face_ = 0;
        chase_position_ = 0;
        base_hue_ = 0;
        pulse_timer_ = 0.0f;
        is_pulsing_ = false;
        // Clear LEDs on reset
        PixelTheater::fill_solid(leds, PixelTheater::CRGB::Black);
    }

    void tick() override {
        PixelTheater::Scene::tick(); 
        float dt = deltaTime();
        phase_timer_ += dt;

        // --- Phase Logic --- 
        switch (current_phase_) {
            case FADE_IN:
                runFadeIn(dt);
                if (phase_timer_ >= DURATION_FADE_IN) nextPhase();
                break;
            case FACE_CYCLE:
                runFaceCycle(dt);
                if (phase_timer_ >= DURATION_FACE_CYCLE) nextPhase();
                break;
            case LED_CHASE:
                runLedChase(dt);
                if (phase_timer_ >= DURATION_LED_CHASE) nextPhase();
                break;
            case BRIGHTNESS_PULSE:
                runBrightnessPulse(dt);
                if (phase_timer_ >= DURATION_BRIGHTNESS_PULSE) nextPhase();
                break;
             case RAINBOW_CYCLE:
                runRainbowCycle(dt);
                if (phase_timer_ >= DURATION_RAINBOW_CYCLE) nextPhase(); // Loop back or end
                break;
            default:
                // Should not happen
                nextPhase(); 
                break;
        }
    }

    void nextPhase() {
        current_phase_ = static_cast<TestPhase>((current_phase_ + 1) % NUM_PHASES);
        phase_timer_ = 0.0f; 
        // Reset phase-specific state if necessary
        is_pulsing_ = false; 
        pulse_timer_ = 0.0f;
        // Clear LEDs between phases for clarity
        PixelTheater::fill_solid(leds, PixelTheater::CRGB::Black);
        logInfo("Entering Phase: %d", current_phase_);
    }

    // --- Phase Implementations --- 

    void runFadeIn(float dt) {
        // Simple fade in - already handled by clearing initially and letting next phase draw
        // Or, add a slow brightness ramp here if desired.
        float brightness_factor = std::min(1.0f, phase_timer_ / DURATION_FADE_IN);
        uint8_t brightness = brightness_factor * MAX_BRIGHTNESS;
        PixelTheater::fill_solid(leds, PixelTheater::CHSV(0, 0, brightness)); // Fade white in
    }

    void runFaceCycle(float dt) {
        float time_per_face = DURATION_FACE_CYCLE / model().faceCount();
        current_face_ = static_cast<int>(phase_timer_ / time_per_face) % model().faceCount();
        uint8_t face_hue = (current_face_ * 255) / model().faceCount(); 
        PixelTheater::CHSV face_color_hsv = PixelTheater::CHSV(face_hue, 255, MAX_BRIGHTNESS);
        PixelTheater::CRGB face_color_rgb = face_color_hsv;

        // Clear all LEDs first
        PixelTheater::fill_solid(leds, PixelTheater::CRGB::Black);
        
        // Light up LEDs belonging to the current face
        for (size_t i = 0; i < ledCount(); ++i) {
            if (model().point(i).face_id() == current_face_) {
                leds[i] = face_color_rgb;
            }
        }
    }

    void runLedChase(float dt) {
        const int trail_length = 5;
        const float leds_per_second = 200.0f; // Adjust speed as needed
        int leds_to_advance = static_cast<int>(leds_per_second * dt);
        if (leds_to_advance < 1) leds_to_advance = 1; // Ensure progress

        // Fade existing LEDs slightly
        for(size_t i = 0; i < ledCount(); ++i) {
            leds[i].fadeToBlackBy(40);
        }

        base_hue_ += 1; // Slowly cycle hue

        // Draw the head of the chase
        for(int i=0; i < leds_to_advance; ++i) {
            int current_pos = (chase_position_ + i) % ledCount();
            leds[current_pos] = PixelTheater::CHSV(base_hue_, 255, MAX_BRIGHTNESS);
        }
        chase_position_ = (chase_position_ + leds_to_advance) % ledCount();
    }

    void runBrightnessPulse(float dt) {
        pulse_timer_ += dt;
        uint8_t current_brightness = MAX_BRIGHTNESS;

        // Check if it's time to start a pulse
        if (!is_pulsing_ && pulse_timer_ >= PULSE_INTERVAL) {
            is_pulsing_ = true;
            pulse_timer_ = 0.0f; // Reset timer for pulse duration
        }

        // Handle pulsing state
        if (is_pulsing_) {
            if (pulse_timer_ <= PULSE_DURATION) {
                // Ramp up quickly to max brightness (optional, could just snap)
                // float pulse_factor = pulse_timer_ / (PULSE_DURATION * 0.2f); // Quick ramp up
                // current_brightness = MAX_BRIGHTNESS + (PULSE_BRIGHTNESS - MAX_BRIGHTNESS) * std::min(1.0f, pulse_factor);
                current_brightness = PULSE_BRIGHTNESS; // Snap to full bright
            } else {
                 // Fade back down after pulse duration
                 float fade_factor = (pulse_timer_ - PULSE_DURATION) / (PULSE_INTERVAL * 0.1f); // Faster fade down
                 current_brightness = PULSE_BRIGHTNESS - (PULSE_BRIGHTNESS - MAX_BRIGHTNESS) * std::min(1.0f, fade_factor);
                 if (current_brightness <= MAX_BRIGHTNESS) {
                     current_brightness = MAX_BRIGHTNESS;
                     is_pulsing_ = false; // End pulse state
                     pulse_timer_ = 0.0f; // Reset timer for interval
                 }
            }
        }

        // Set all LEDs to the base color with current brightness
        PixelTheater::CHSV base_color_hsv(160, 200, current_brightness); // Blue-ish base
        PixelTheater::fill_solid(leds, PixelTheater::CRGB(base_color_hsv));
    }

     void runRainbowCycle(float dt) {
        base_hue_ += 2; // Speed of hue shift
        uint8_t delta_hue = 255 / std::max(1, (int)ledCount() / 10); // Spread rainbow

        for(size_t i=0; i < ledCount(); ++i) {
            leds[i] = PixelTheater::CHSV(base_hue_ + i * delta_hue, 255, MAX_BRIGHTNESS);
        }
    }

    // The Scene class might not have a status() method to override
    // Let's provide it without the override keyword
    std::string status() const {
        std::string phase_name = "Unknown";
        switch(current_phase_) {
            case FADE_IN: phase_name = "Fade In"; break;
            case FACE_CYCLE: phase_name = "Face Cycle"; break;
            case LED_CHASE: phase_name = "LED Chase"; break;
            case BRIGHTNESS_PULSE: phase_name = "Brightness Pulse"; break;
            case RAINBOW_CYCLE: phase_name = "Rainbow Cycle"; break;
            default: break;
        }
        return "Diagnostic Test Phase: " + phase_name + 
               " (" + std::to_string((int)phase_timer_) + "s)";
    }
};

} // namespace Scenes 