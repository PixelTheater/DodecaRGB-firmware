#pragma once
#include <map>
#include "animation.h"

class AnimationBuilder {
    using CreatorFunc = std::unique_ptr<Animation>(*)();
    static std::map<String, CreatorFunc>& registry();
    
public:
    static void registerAnimation(const String& name, CreatorFunc creator);
    static std::unique_ptr<Animation> create(const String& name);
}; 