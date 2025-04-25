#include "sparkles_scene.h"
#include "PixelTheater/SceneKit.h" // Access common scene elements
#include <cmath>                     // For std::sin, std::fmod, std::max, std::round
#include <algorithm>                 // For std::clamp, std::max
#include <vector>                    // For std::max used with initializer list
// #include <string.h> // Not needed for loop-based copy
// #include <avr/pgmspace.h> // Alternative header for PROGMEM utilities if string.h doesn't work
// #include <FastLED.h> // Likely redundant via SceneKit

namespace Scenes {

// --- Simplified Constants --- 
// Parameter Defaults REMOVED - Defined in header now
/*
static constexpr float DEFAULT_SPEED = 0.4f;         
static constexpr float DEFAULT_GLITTER = 0.5f;       
static constexpr float DEFAULT_CHAOS = 0.4f;         
static constexpr float DEFAULT_INTENSITY = 0.5f;     
*/
// Core Timing
static constexpr float MIN_TRANSITION_S = 1.0f;  
static constexpr float MAX_TRANSITION_S = 60.0f; 
static constexpr float MIN_TARGET_MIX_FREQ = 0.05f; // Base freq range for targets
static constexpr float MAX_TARGET_MIX_FREQ = 0.8f;  
// Core Chaos Scaling
static constexpr float MAX_CHAOS_DURATION_SCALE = 3.0f; 
static constexpr float MAX_CHAOS_FREQ_SCALE = 2.0f;     // How much currentChaosLevel affects target freq randomization
// Smoothing / Lerp Rates
static constexpr float MIX_RATIO_LERP_RATE = 0.5f;  // Rate per second
static constexpr float MIX_FREQ_LERP_RATE = 0.3f;   // Rate per second
static constexpr float CHAOS_LEVEL_LERP_RATE = 0.1f; // Rate per second (slow drift)
// Other constants ...
static constexpr float MIN_RANDOMIZED_DURATION_S = 0.2f; 
static constexpr uint8_t BASE_FADE = 20;
static constexpr float FADE_INTENSITY_SCALE = 15.0f; 
static constexpr uint8_t BASE_SPARKLE_STRENGTH = 64;
static constexpr float SPARKLE_INTENSITY_SCALE = 191.0f; 
static constexpr float SPARKLE_DENSITY_FACTOR = 0.5f;
static constexpr float GLITTER_MIX_AMPLITUDE_SCALE = 0.5f;
static constexpr float CHAOS_MIX_NOISE_SCALE = 0.05f; // Reduced noise slightly
static constexpr float BRIGHTNESS_TIME_SCALE = 10000.0f;
static constexpr float BRIGHTNESS_FREQ = 0.5f;
static constexpr uint8_t BASE_SPARKLE_BRIGHTNESS = 100;
static constexpr float GLITTER_BRIGHTNESS_SCALE = 220.0f;


// === Helper Implementations ===

// Updated startNewColorTransition
void SparklesScene::startNewColorTransition(float speed, float chaosParam) {
    previousColorATarget = colorATarget;
    previousColorBTarget = colorBTarget;
    colorATarget = selectNewColorFromPalette(palette1);
    colorBTarget = selectNewColorFromPalette(palette2);

    // --- Update Targets based on Params and current Chaos --- 
    // Target Chaos Level (influenced by chaosParam)
    targetChaosLevel_ = randomFloat(0.0f, chaosParam); // Target drifts towards value set by param

    // Target Mix Ratio (random jump)
    targetMixRatio_ = randomFloat(); 

    // Target Mix Frequency (base from Speed, randomized by currentChaosLevel_)
    float normSpeed = std::clamp(speed, 0.0f, 1.0f);
    float baseTargetFreq = map(normSpeed, 0.0f, 1.0f, MIN_TARGET_MIX_FREQ, MAX_TARGET_MIX_FREQ);
    float freqChaosScaleMax = 1.0f + currentChaosLevel_ * (MAX_CHAOS_FREQ_SCALE - 1.0f);
    float freqChaosScaleMin = 1.0f / freqChaosScaleMax;
    targetMixOscillationFreq_ = baseTargetFreq * randomFloat(freqChaosScaleMin, freqChaosScaleMax);
    targetMixOscillationFreq_ = std::max(0.01f, targetMixOscillationFreq_); // Ensure positive freq

    // Calculate next Color Transition Durations (base from Speed, randomized by currentChaosLevel_)
    float baseDuration = calculateBaseTransitionDuration(speed);
    colorATransitionDuration = randomizeDuration(baseDuration, currentChaosLevel_); // Use evolving chaos level
    colorBTransitionDuration = randomizeDuration(baseDuration, currentChaosLevel_); 

    colorChangeTimer = std::max(colorATransitionDuration, colorBTransitionDuration);
    is_initial_transition = false; 
}

// calculateBaseTransitionDuration uses speed param - OK
// randomizeDuration uses currentChaosLevel_ passed in - OK
float SparklesScene::randomizeDuration(float baseDuration, float currentChaosLevel) { // Signature updated
    float normChaos = std::clamp(currentChaosLevel, 0.0f, 1.0f);
    float maxScale = 1.0f + normChaos * (MAX_CHAOS_DURATION_SCALE - 1.0f); 
    float minScale = 1.0f / maxScale;
    float randomMultiplier = randomFloat(minScale, maxScale);
    float randomizedDuration = baseDuration * randomMultiplier;
    return std::max(MIN_RANDOMIZED_DURATION_S, randomizedDuration);
}

CRGB SparklesScene::selectNewColorFromPalette(const CRGBPalette16& pal) {
    return colorFromPalette(pal, random8());
}

float SparklesScene::calculateBaseTransitionDuration(float speed) const {
    // Map speed [0, 1] -> duration [MAX, MIN] 
    float normSpeed = std::clamp(speed, 0.0f, 1.0f);
    // Apply ease-out quad to make lower speeds have longer durations more dramatically
    float speedFactor = outQuadF(1.0f - normSpeed); // Ease (1-speed)
    float duration = map(speedFactor, 0.0f, 1.0f, MIN_TRANSITION_S, MAX_TRANSITION_S);
    return duration;
}

CRGB SparklesScene::lerpColor(const CRGB& start, const CRGB& end, float factor) const {
    uint8_t t = static_cast<uint8_t>(std::round(factor * 255.0f));
    return CRGB(
        lerp8by8(start.r, end.r, t),
        lerp8by8(start.g, end.g, t),
        lerp8by8(start.b, end.b, t)
    );
}

uint8_t SparklesScene::calculateFadeAmount(float intensity, float glitter) const {
    float combinedFactor = (intensity + glitter) / 2.0f;
    uint8_t fadeAmount = BASE_FADE + static_cast<uint8_t>(combinedFactor * FADE_INTENSITY_SCALE);
    return std::min(fadeAmount, (uint8_t)250);
}

uint8_t SparklesScene::calculateSparkleStrength(float intensity) const {
    return BASE_SPARKLE_STRENGTH + static_cast<uint8_t>(intensity * SPARKLE_INTENSITY_SCALE);
}

uint8_t SparklesScene::calculateSparkleBrightness(float glitter) {
    float time = millis() / BRIGHTNESS_TIME_SCALE;
    float brightnessFactor = 0.5f + 0.5f * std::sin(time * BRIGHTNESS_FREQ + mixOscillatorPhase);
    return BASE_SPARKLE_BRIGHTNESS + static_cast<uint8_t>(glitter * (brightnessFactor - 0.5f) * GLITTER_BRIGHTNESS_SCALE);
}

// === Lifecycle Methods ===

void SparklesScene::setup() {
    // --- Define Metadata ---
    set_name("Sparkles");
    set_author("Somebox");
    set_description("Shimmering sparkles with transitioning colors.");
    set_version("2.1");

    // --- Define Parameters (using static constexpr defaults) ---
    param("Speed", "ratio", 0.0f, 1.0f, SparklesScene::DEFAULT_SPEED, "clamp", "Avg speed of changes (0=Slow, 1=Fast)");
    param("Glitter", "ratio", SparklesScene::DEFAULT_GLITTER, "clamp", "Sparkle brightness variance & Mix range");
    param("Chaos", "ratio", SparklesScene::DEFAULT_CHAOS, "clamp", "Max randomness/entropy level (0=Calm, 1=Wild)");
    param("Intensity", "ratio", SparklesScene::DEFAULT_INTENSITY, "clamp", "Sparkle density & Inverse fade");

    // --- Initialize Palettes ---
    // Use PixelTheater platform-independent palettes
    for (int i = 0; i < 16; ++i) {
        // Assuming PixelTheater::Palettes::CloudColors is a CRGBPalette16
        palette1[i] = PixelTheater::Palettes::CloudColors[i];
        palette2[i] = PixelTheater::Palettes::HeatColors[i];
    }

    // --- Initialize State ---
    colorA = CRGB::Black; colorB = CRGB::Black;
    targetMixRatio_ = 0.5f;
    currentMixRatio_ = 0.5f;
    targetMixOscillationFreq_ = map(DEFAULT_SPEED, 0.0f, 1.0f, MIN_TARGET_MIX_FREQ, MAX_TARGET_MIX_FREQ);
    mixOscillationFreq_ = targetMixOscillationFreq_;
    targetChaosLevel_ = DEFAULT_CHAOS; // Start targeting default chaos
    currentChaosLevel_ = 0.0f; // Start calm
    is_initial_transition = true;
    mixOscillatorPhase = randomFloat(0.0f, PT_TWO_PI);

    // Call startNewColorTransition to set initial targets and durations
    startNewColorTransition(settings["Speed"], settings["Chaos"]);
    // Override timer for initial fade-in feel using eased progress
    colorChangeTimer = std::max(colorATransitionDuration, colorBTransitionDuration);
    // Set previous targets for first interpolation correctly
    previousColorATarget = CRGB::Black; 
    previousColorBTarget = CRGB::Black;
    
    logInfo("SparklesScene setup complete");
}

void SparklesScene::tick() {
    float dt = deltaTime();
    // Read Params
    float speed = settings["Speed"];
    float glitter = settings["Glitter"];
    float chaosParam = settings["Chaos"]; // User control over target chaos range
    float intensity = settings["Intensity"];

    // 1. Lerp Internal State Variables
    currentChaosLevel_ += (targetChaosLevel_ - currentChaosLevel_) * CHAOS_LEVEL_LERP_RATE * dt;
    mixOscillationFreq_ += (targetMixOscillationFreq_ - mixOscillationFreq_) * MIX_FREQ_LERP_RATE * dt;
    currentMixRatio_ += (targetMixRatio_ - currentMixRatio_) * MIX_RATIO_LERP_RATE * dt;

    // 2. Update Mix Oscillator Phase
    mixOscillatorPhase = std::fmod(mixOscillatorPhase + mixOscillationFreq_ * dt, PT_TWO_PI);

    // 3. Handle Color Transition Timing & Target Updates
    colorChangeTimer -= dt;
    if (colorChangeTimer <= 0) {
        startNewColorTransition(speed, chaosParam);
    }

    // 4. Calculate Color Transition Progress & Interpolate
    float timeRemaining = std::max(0.0f, colorChangeTimer);
    float progressA = (colorATransitionDuration > 1e-6f) ? 1.0f - std::clamp(timeRemaining / colorATransitionDuration, 0.0f, 1.0f) : 1.0f;
    float progressB = (colorBTransitionDuration > 1e-6f) ? 1.0f - std::clamp(timeRemaining / colorBTransitionDuration, 0.0f, 1.0f) : 1.0f;
    if (is_initial_transition) {
        progressA = 1.0f - std::pow(1.0f - progressA, 3.0f);
        progressB = 1.0f - std::pow(1.0f - progressB, 3.0f);
    }
    colorA = lerpColor(previousColorATarget, colorATarget, progressA);
    colorB = lerpColor(previousColorBTarget, colorBTarget, progressB);

    // 5. Global Fade
    uint8_t fade = calculateFadeAmount(intensity, glitter);
    size_t count = ledCount();
    for(size_t i = 0; i < count; ++i) {
        leds[i].fadeToBlackBy(fade); // Use CRGB method via leds proxy
    }

    // 6. Pixel Sparkles
    uint16_t numTotalSparkles = static_cast<uint16_t>(intensity * ledCount() * SPARKLE_DENSITY_FACTOR); 
    uint8_t sparkleStrength = calculateSparkleStrength(intensity);
    // Use lerped currentMixRatio_ for distribution
    float actualMixRatio = currentMixRatio_;
    // Add noise based on currentChaosLevel_?
    actualMixRatio += randomFloat(-currentChaosLevel_ * CHAOS_MIX_NOISE_SCALE, currentChaosLevel_ * CHAOS_MIX_NOISE_SCALE);
    actualMixRatio = std::clamp(actualMixRatio, 0.0f, 1.0f);

    uint16_t numSparklesA = static_cast<uint16_t>(std::round(numTotalSparkles * actualMixRatio));
    uint16_t numSparklesB = static_cast<uint16_t>(std::round(numTotalSparkles * (1.0f - actualMixRatio)));

    // Add Sparkles A
    for (uint16_t i = 0; i < numSparklesA; ++i) {
        uint16_t px = random16() % ledCount();
        uint8_t brightnessVariation = calculateSparkleBrightness(glitter);
        CRGB variedColorA = colorA;
        variedColorA.nscale8(brightnessVariation); // Apply brightness variation first
        leds[px] += variedColorA.nscale8(sparkleStrength); // Add scaled color
    }

    // Add Sparkles B
    for (uint16_t i = 0; i < numSparklesB; ++i) {
        uint16_t px = random16() % ledCount();
        uint8_t brightnessVariation = calculateSparkleBrightness(glitter);
        CRGB variedColorB = colorB;
        variedColorB.nscale8(brightnessVariation); // Apply brightness variation first
        leds[px] += variedColorB.nscale8(sparkleStrength); // Add scaled color
    }
}

std::string SparklesScene::status() const {
    char buffer[200];
    snprintf(buffer, sizeof(buffer), 
             "Tmr:%.1f/%.1f|Chaos:%.2f->%.2f|Mix:%.2f->%.2f|Freq:%.2f->%.2f|ClrA:%02X%02X%02X|ClrB:%02X%02X%02X",
             colorChangeTimer, std::max(colorATransitionDuration, colorBTransitionDuration),
             currentChaosLevel_, targetChaosLevel_,
             currentMixRatio_, targetMixRatio_,
             mixOscillationFreq_, targetMixOscillationFreq_,
             colorA.r, colorA.g, colorA.b,
             colorB.r, colorB.g, colorB.b);
    return std::string(buffer);
}

} // namespace Scenes