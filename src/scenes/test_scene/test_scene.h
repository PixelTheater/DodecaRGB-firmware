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
    const float DURATION_FADE_IN = 3.0f;
    const float DURATION_FACE_CYCLE = 20.0f;
    const float DURATION_LED_CHASE = 20.0f;
    const float DURATION_BRIGHTNESS_PULSE = 20.0f;
    const float DURATION_RAINBOW_CYCLE = 20.0f;
    const float FACE_TRANSITION_DURATION = 0.5f; // Duration of crossfade between faces

    // Constants for the pulse effect
    static constexpr float PULSE_INTERVAL = 3.0f; // Time between starts of pulses (seconds)
    static constexpr float PULSE_DURATION = 0.3f; // Total duration of the pulse (seconds) - controls speed

    // Define start and end colors for the pulse
    const PixelTheater::CRGB START_COLOR = PixelTheater::CRGB(0, 0, 50); // Deep dark blue
    const PixelTheater::CRGB END_COLOR = PixelTheater::CRGB::White;      // Bright white

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
        uint8_t brightness = brightness_factor * 255;
        PixelTheater::fill_solid(leds, PixelTheater::CHSV(0, 0, brightness)); // Fade white in
    }

    void runFaceCycle(float dt) {
        // Prevent division by zero if faceCount is 0
        if (model().faceCount() == 0) return;

        float time_per_face = DURATION_FACE_CYCLE / model().faceCount();
        // Ensure transition is not longer than time spent on face
        float effective_transition_duration = std::min(time_per_face, FACE_TRANSITION_DURATION);

        int current_face_idx = static_cast<int>(phase_timer_ / time_per_face) % model().faceCount();
        int next_face_idx = (current_face_idx + 1) % model().faceCount();
        float time_on_current_face = fmod(phase_timer_, time_per_face);

        // Calculate colors for current and next faces
        uint8_t current_face_hue = (current_face_idx * 255) / model().faceCount();
        PixelTheater::CRGB current_face_color = PixelTheater::CHSV(current_face_hue, 255, 255);

        uint8_t next_face_hue = (next_face_idx * 255) / model().faceCount();
        PixelTheater::CRGB next_face_color = PixelTheater::CHSV(next_face_hue, 255, 255);

        uint8_t current_face_brightness = 255;
        uint8_t next_face_brightness = 0;

        // Check if we are in the transition period at the end of the face display time
        float transition_start_time = time_per_face - effective_transition_duration;
        if (time_on_current_face >= transition_start_time && effective_transition_duration > 1e-6f) {
            float transition_progress = (time_on_current_face - transition_start_time) / effective_transition_duration;
            transition_progress = std::clamp(transition_progress, 0.0f, 1.0f);

            // Linear crossfade: current fades out (255 -> 0), next fades in (0 -> 255)
            current_face_brightness = static_cast<uint8_t>((1.0f - transition_progress) * 255.0f);
            next_face_brightness = static_cast<uint8_t>(transition_progress * 255.0f);
        }

        // Clear all LEDs first
        PixelTheater::fill_solid(leds, PixelTheater::CRGB::Black);
        
        // Light up LEDs belonging to the current face
        for (size_t i = 0; i < ledCount(); ++i) {
            int led_face_id = model().point(i).face_id();

            if (led_face_id == current_face_idx && current_face_brightness > 0) {
                leds[i] = current_face_color;
                leds[i].nscale8(current_face_brightness); // Apply fade out scale
            } else if (led_face_id == next_face_idx && next_face_brightness > 0) {
                // Only light up next face during transition (or if brightness > 0)
                leds[i] = next_face_color;
                leds[i].nscale8(next_face_brightness); // Apply fade in scale
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
            leds[current_pos] = PixelTheater::CHSV(base_hue_, 255, 255);
        }
        chase_position_ = (chase_position_ + leds_to_advance) % ledCount();
    }

    void runBrightnessPulse(float dt) {
        pulse_timer_ += dt;

        PixelTheater::CRGB current_color = START_COLOR;

        // Check if it's time to start a pulse
        if (!is_pulsing_ && pulse_timer_ >= PULSE_INTERVAL) {
            is_pulsing_ = true;
            pulse_timer_ = 0.0f; // Reset timer for pulse duration
        }

        // Handle pulsing state
        if (is_pulsing_) {
            if (pulse_timer_ >= PULSE_DURATION) {
                // Pulse finished, reset state
                current_color = START_COLOR;
                is_pulsing_ = false;
                pulse_timer_ = 0.0f; // Reset timer for next interval
            } else {
                // Calculate blend amount (0-255) for ramp up/down
                float progress = pulse_timer_ / PULSE_DURATION; // 0.0 to 1.0
                float blend_factor = 0.0f;
                if (progress < 0.5f) {
                    // Ramp up phase (0.0 to 0.5 progress -> 0.0 to 1.0 factor)
                    blend_factor = progress * 2.0f;
                } else {
                    // Ramp down phase (0.5 to 1.0 progress -> 1.0 to 0.0 factor)
                    blend_factor = (1.0f - progress) * 2.0f;
                }
                uint8_t blend_amount = static_cast<uint8_t>(std::clamp(blend_factor * 255.0f, 0.0f, 255.0f));

                // Blend between START_COLOR and END_COLOR
                current_color = PixelTheater::blend(START_COLOR, END_COLOR, blend_amount);
            }
        }

        // Set all LEDs to the base color with current brightness
        PixelTheater::fill_solid(leds, current_color);
    }

     void runRainbowCycle(float dt) {
        // Get elapsed time in this phase for animation
        float time = phase_timer_;
        const float base_speed = 30.0f; // Base animation speed factor

        for(size_t i=0; i < ledCount(); ++i) {
            int face_id = model().point(i).face_id();

            if (face_id >= 0) {
                // Deterministically calculate animation parameters based on face_id
                float speed_mod = 1.0f + fmodf(face_id, 5.0f) * 0.3f; // Speed varies slightly per face (0-4 pattern)
                float direction = (face_id % 2 == 0) ? 1.0f : -1.0f;    // Direction alternates per face
                float face_base_hue = fmodf(face_id * (255.0f / std::max(1, (int)model().faceCount())), 255.0f);

                // Calculate current hue, wrapping around 0-255
                float hue = face_base_hue + direction * speed_mod * time * base_speed;
                hue = fmodf(hue/10.0f, 255.0f);
                if (hue < 0.0f) {
                    hue += 255.0f; // Ensure hue is positive
                }

                // Set LED color using CHSV
                leds[i] = PixelTheater::CHSV(static_cast<uint8_t>(hue), 255, 255);
            } else {
                // LEDs not belonging to a face are off
                leds[i] = PixelTheater::CRGB::Black;
            }
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