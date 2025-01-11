#include "animation_builder.h"
#include "animations/blob.h"
#include "animations/sparkles.h"

// Static registry implementation
std::map<String, AnimationBuilder::CreatorFunc>& AnimationBuilder::registry() {
    static std::map<String, CreatorFunc> impl;
    return impl;
}

// Registration method
void AnimationBuilder::registerAnimation(const String& name, CreatorFunc creator) {
    registry()[name] = creator;
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