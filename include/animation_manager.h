#pragma once
#include <memory>
#include <vector>
#include <FastLED.h>
#include "animation.h"
#include "animation_builder.h"


// Macro to make registration easier
#define REGISTER_ANIMATION(className) \
  animations.add(std::make_unique<className>())

enum class PlaybackMode {
    HOLD,           // Stay on current animation until manual advance
    ADVANCE,        // Automatically advance through animations
    RANDOM         // Randomly select next animation
};

class AnimationManager {
private:
  std::vector<std::unique_ptr<Animation>> animations;
  size_t current_index = 0;
  Animation* _currentAnimation = nullptr;
  CRGB* leds;
  const uint8_t num_sides;
  uint16_t leds_per_side;
  PlaybackMode _playbackMode = PlaybackMode::HOLD;
  float _holdTime = 0.0f;  // Time in seconds between animations (0 = manual advance)
  unsigned long _lastSwitchTime = 0;

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
      if (_currentAnimation == nullptr) {
        _currentAnimation = animations.back().get();
      }
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

  String getCurrentAnimationName() const {
    if (animations.empty()) return "No animations";
    return animations[current_index]->getName();
  }

  size_t getCurrentAnimationIndex() const { return current_index; }

  size_t getPlaylistLength() const { return animations.size(); }

  void setCurrentAnimation(const std::string& name);
  void setCurrentAnimation(size_t index);

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
    if (_playbackMode != PlaybackMode::HOLD) {
        if (_holdTime > 0 && (millis() - _lastSwitchTime) >= (_holdTime * 1000)) {
            if (_playbackMode == PlaybackMode::ADVANCE) {
                nextAnimation();
            } else { // RANDOM
                randomAnimation();
            }
            _lastSwitchTime = millis();
        }
    }
    
    if (_currentAnimation) {
        _currentAnimation->tick();
    }
  }

  void nextAnimation() {
    if (!animations.empty()) {
        current_index = (current_index + 1) % animations.size();
        _currentAnimation = animations[current_index].get();
    }
  }

  void setPlaybackMode(PlaybackMode mode, float holdTimeSeconds = 0.0f) {
        _playbackMode = mode;
        _holdTime = holdTimeSeconds;
        _lastSwitchTime = millis();
    }

  void randomAnimation() {
    if (animations.empty()) return;
    
    // If we only have one animation, nothing to randomize
    if (animations.size() == 1) {
        setCurrentAnimation(0);
        return;
    }
    
    // Pick a random index that's different from current
    size_t newIndex;
    do {
        newIndex = random(0, animations.size());
    } while (newIndex == current_index);
    
    setCurrentAnimation(newIndex);
  }

};

void AnimationManager::setCurrentAnimation(const std::string& name) {
    for (size_t i = 0; i < animations.size(); i++) {
        if (animations[i]->getName() == name) {
            current_index = i;
            return;
        }
    }
}

void AnimationManager::setCurrentAnimation(size_t index) {
    if (index < animations.size()) {
        current_index = index;
        _currentAnimation = animations[current_index].get();
    }
}