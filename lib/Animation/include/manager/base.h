#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "../scene.h"

namespace Animation {

class AnimationManager {
public:
    AnimationManager() = default;
    virtual ~AnimationManager() = default;

    // Scene management
    template<typename T>
    Settings& add(const std::string& name);
    void remove(const std::string& name);
    void clear();

    // Scene access
    Settings& operator[](const std::string& name);
    bool hasScene(const std::string& name) const;
    std::string currentScene() const;

    // Playback control
    void play(const std::string& name);
    void next();
    void random();
    void update();

protected:
    // Allow derived classes to setup scenes
    virtual void setupScene(Scene* /*scene*/) {}

private:
    std::unordered_map<std::string, std::unique_ptr<Scene>> _scenes;
    Scene* _current{nullptr};
};

} // namespace Animation 