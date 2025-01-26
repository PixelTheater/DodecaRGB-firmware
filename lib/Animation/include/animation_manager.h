#pragma once
#include <string>
#include <map>
#include <memory>
#include "scene.h"

namespace Animation {

enum class PlaybackMode {
    HOLD,
    ADVANCE,
    RANDOM
};

class AnimationManager {
public:
    AnimationManager() = default;
    
    // Core interface - move implementations to cpp
    Settings& operator[](const std::string& name);
    void registerAnimation(const std::string& name, std::unique_ptr<Scene> animation);
    Scene* getAnimation(const std::string& name);
    
    // Playback control
    void play(const std::string& name);
    void update();
    void next();
    void random();
    
    // Status reporting
    void setStatusInterval(unsigned long ms) { _status_interval = ms; }
    bool hasStatus() const;
    std::string status() const;

    void setPlaybackMode(PlaybackMode mode, float interval = 0.0f);

private:
    std::map<std::string, std::unique_ptr<Scene>> _animations;
    Scene* _current = nullptr;
    unsigned long _status_interval = 1000;  // Default to 1 second
    unsigned long _last_status = 0;
    
    // Add playback control members
    PlaybackMode _mode = PlaybackMode::HOLD;
    float _interval = 0.0f;
    unsigned long _last_switch = 0;
};

} // namespace Animation 