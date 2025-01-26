#include "animation_manager.h"
#ifdef TEST_BUILD
#include "arduino_mock.h"  // Will be found in test/helpers
#endif

namespace Animation {

Settings& AnimationManager::operator[](const std::string& name) {
    auto it = _animations.find(name);
    if (it == _animations.end()) {
        throw std::invalid_argument("Unknown animation: " + name);
    }
    return it->second->settings();
}

void AnimationManager::registerAnimation(const std::string& name, std::unique_ptr<Scene> animation) {
    _animations[name] = std::move(animation);
}

Scene* AnimationManager::getAnimation(const std::string& name) {
    auto it = _animations.find(name);
    return it != _animations.end() ? it->second.get() : nullptr;
}

void AnimationManager::play(const std::string& name) {
    auto it = _animations.find(name);
    if (it != _animations.end()) {
        _current = it->second.get();
        _current->init();
    }
}

void AnimationManager::update() {
    if (_animations.empty()) return;
    
    if (!_current) {
        _current = _animations.begin()->second.get();
        _current->init();
    }
    
    _current->tick();
    
    // Handle auto-advance/random
    if (_interval > 0) {
        unsigned long now = millis();
        if ((now - _last_switch) >= static_cast<unsigned long>(_interval * 1000)) {
            _last_switch = now;
            switch (_mode) {
                case PlaybackMode::ADVANCE:
                    next();
                    break;
                case PlaybackMode::RANDOM:
                    random();
                    break;
                default:
                    break;
            }
        }
    }
}

void AnimationManager::next() {
    if (_animations.empty()) return;
    
    // Find current animation and advance to next
    for (auto it = _animations.begin(); it != _animations.end(); ++it) {
        if (it->second.get() == _current) {
            ++it;
            if (it == _animations.end()) {
                it = _animations.begin();
            }
            _current = it->second.get();
            _current->init();
            break;
        }
    }
}

void AnimationManager::random() {
    if (_animations.empty()) return;
    
    size_t index = rand() % _animations.size();
    auto it = _animations.begin();
    std::advance(it, index);
    _current = it->second.get();
    _current->init();
}

bool AnimationManager::hasStatus() const {
    if (!_current) return false;
    unsigned long now = millis();
    return (now - _last_status) >= _status_interval;
}

std::string AnimationManager::status() const {
    if (!_current) return "";
    return _current->getStatusBuffer();
}

void AnimationManager::setPlaybackMode(PlaybackMode mode, float interval) {
    _mode = mode;
    _interval = interval;
    _last_switch = millis();
}

} // namespace Animation
