#pragma once
#include <memory>
#include "display.h"
#include "settings.h"

namespace Animation {

class Scene {
public:
    Scene() : _settings() {}  // Default constructor back
    virtual ~Scene() = default;
    
    // Core interface
    virtual void init() {
        setup();     // Defines parameters
        reset();     // Initialize state
    }

protected:
    // Required methods
    virtual void setup() = 0;
    virtual void reset() = 0;
    virtual void tick() = 0;
    
    // Optional methods - declare without implementation
    virtual void onSettingsChanged();
    virtual void status();
    
    // Parameter helpers
    ParamBuilder& param(const std::string& name) {
        return _settings.param(name);
    }
    
    // Settings access methods
    Settings& settings() { return _settings; }
    const Settings& settings() const { return _settings; }
    
    float settings(const std::string& name) const {
        return _settings(name);
    }
    
    template<typename T>
    const T& settings(const std::string& name) const {
        return _settings.operator()<T>(name);
    }
    
    template<typename T>
    const T& get(const std::string& name) {
        return _settings.getInstance<T>(name);
    }

    // Status buffer access
    std::string getStatusBuffer() const { return _status_buffer; }

private:
    Settings _settings;
    std::string _status_buffer;  // For status output
    friend class AnimationManager;  // For access to settings
};

} // namespace Animation 