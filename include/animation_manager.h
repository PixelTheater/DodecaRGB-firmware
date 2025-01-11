#pragma once
#include <memory>
#include <vector>
#include <FastLED.h>
#include "animation.h"


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

  void add(std::unique_ptr<Animation> animation, const AnimParams& params = AnimParams()) {
    animation->configure(leds, num_sides, leds_per_side);
    animation->init(params);
    animations.push_back(std::move(animation));
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

  // ... rest of AnimationManager methods ...
};