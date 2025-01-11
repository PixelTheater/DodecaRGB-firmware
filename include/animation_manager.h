#pragma once
#include <memory>
#include <vector>
#include <FastLED.h>
#include "animation.h"
#include "animation_builder.h"


// Macro to make registration easier
#define REGISTER_ANIMATION(className) \
  animations.add(std::make_unique<className>())

class AnimationManager {
private:
  std::vector<std::unique_ptr<Animation>> animations;
  size_t current_index = 0;
  CRGB* leds;
  const uint8_t num_sides;
  uint16_t leds_per_side;

public:
  AnimationManager(CRGB* leds, uint16_t num_leds, uint8_t num_sides) 
    : leds(leds)
    , num_sides(num_sides) {
        leds_per_side = num_leds / num_sides;
    }

  void add(const String& name) {
    auto anim = AnimationBuilder::create(name);
    if (anim) {
      auto params = anim->getDefaultParams();
      anim->init(params);
      anim->configure(leds, points, num_sides, leds_per_side);
      animations.push_back(std::move(anim));
    }
  }

  Animation* getCurrentAnimation() {
    if (animations.empty()) return nullptr;
    return animations[current_index].get();
  }

  String getCurrentStatus() const {
    if (animations.empty()) return "No animations";
    return animations[current_index]->getStatus();
  }

  void setCurrentAnimation(size_t index) {
    if (index < animations.size()) {
      current_index = index;
    }
  }

  bool preset(const String& anim_name, const String& preset_name) {
    for (auto& anim : animations) {
      if (String(anim->getName()) == anim_name) {
        auto params = anim->getPreset(preset_name);
        anim->init(params);
        return true;
      }
    }
    Serial.printf("Warning: Animation '%s' not found\n", anim_name.c_str());
    return false;
  }

  void update() {
    if (auto current = getCurrentAnimation()) {
      current->tick();
    }
  }
};