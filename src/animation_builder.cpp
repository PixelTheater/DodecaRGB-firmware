#include "animation.h"
#include "animation_builder.h"
#include "animations/blob.h"
#include "animations/sparkles.h"
#include "animations/xyz_scanner.h"
#include "animations/wandering_particles.h"
#include "animations/geography.h"
#include "animations/color_show.h"
#include "animations/orientation_demo.h"

// Define the static member with default value
uint8_t Animation::global_brightness = 128;  // Start at 50% brightness

// Static registry implementation
std::map<String, AnimationBuilder::CreatorFunc>& AnimationBuilder::registry() {
    static std::map<String, CreatorFunc> impl;
    return impl;
}

// Registration method
void AnimationBuilder::registerAnimation(const String& name, CreatorFunc creator) {
    if (name.length() == 0) {
        Serial.println("Error: Cannot register animation with empty name");
        return;
    }
    
    auto it = registry().find(name);
    if (it != registry().end()) {
        Serial.printf("Error: Animation '%s' already registered\n", name.c_str());
        return;
    }
    
    registry()[name] = creator;
    Serial.printf("Registered animation: %s\n", name.c_str());
}

// Creation method
std::unique_ptr<Animation> AnimationBuilder::create(const String& name) {
    auto it = registry().find(name);
    if (it != registry().end()) {
        return it->second();
    }
    Serial.printf("Warning: Animation '%s' not found\n", name.c_str());
    return nullptr;
}

// Registration helper macro
#define REGISTER_ANIMATION(name, class_name) \
    namespace { \
        static bool registered_##class_name = \
            (AnimationBuilder::registerAnimation(name, \
                []() -> std::unique_ptr<Animation> { return std::make_unique<class_name>(); }), true); \
    }

// Register all animations here
REGISTER_ANIMATION("blobs", BlobAnimation)
REGISTER_ANIMATION("sparkles", Sparkles)
REGISTER_ANIMATION("xyz_scanner", XYZScanner)
REGISTER_ANIMATION("wandering_particles", WanderingParticles) 
REGISTER_ANIMATION("geography", Geography)
REGISTER_ANIMATION("colorshow", ColorShow)
REGISTER_ANIMATION("orientation_demo", OrientationDemo)