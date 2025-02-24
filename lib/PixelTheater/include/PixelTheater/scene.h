#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include "settings.h"
#include "settings_proxy.h"
#include "params/param_def.h"
#include "params/param_value.h"
#include "model/model.h"
#include "stage.h"

namespace PixelTheater {

// Scene - A single animation running on a Stage, with its own parameters and state
//  - Serves as a base class that can be extended by users of the library for their own scenes
//  - lifecycle management (setup, tick, reset, etc)
//  - Manages its own parameters (via Settings)

/* see creating_animations.md for more information */

template<typename ModelDef>
class Scene {
public:
    Scene(Stage<ModelDef>& stage_ref, const ParamDef* params = nullptr, size_t param_count = 0,
          const ParamDef::Metadata* metadata = nullptr)
        : stage(stage_ref)
        , _settings_storage()
        , settings(_settings_storage)
    {
        if (params) {
            _settings_storage = Settings(params, param_count);
        } else {
            init_params();
        }
        _metadata = metadata;
    }

    virtual ~Scene() = default;

    // Prevent copying
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    /**
     * Initialize scene state
     * Called once when scene becomes active
     */
    virtual void setup() = 0;

    /**
     * Update animation state
     * Called every frame (50fps+)
     */
    virtual void tick() {
        _tick_count++;
    }

    /**
     * Reset scene to initial state
     * Optional override
     */
    virtual void reset() {
        _tick_count = 0;
        _settings_storage.reset_all();
    }

    /**
     * Get scene name
     * @return Name of the scene
     */
    const char* name() const {
        return _metadata ? _metadata->name : "Unnamed Scene";
    }

    /**
     * Get scene description
     * @return Description of the scene
     */
    const char* description() const {
        return _metadata ? _metadata->description : "";
    }

    /**
     * Get number of ticks processed
     * @return Tick count
     */
    size_t tick_count() const { return _tick_count; }

    Stage<ModelDef>& stage;
    Settings _settings_storage;
    SettingsProxy settings;      // Now public - main interface for parameter access

protected:
    /**
     * Helper to define parameters in setup()
     */
    void param(const std::string& name, const std::string& type,
              const ParamValue& default_val, const std::string& flags = "") {
        _settings_storage.add_parameter_from_strings(name, type, default_val, flags);
    }

    /**
     * Configure scene parameters
     * Called during initialization if no generated params
     */
    virtual void config() {}

    // Convenience overloads
    void param(const std::string& name, const std::string& type,
              float default_val, const std::string& flags = "") {
        param(name, type, ParamValue(default_val), flags);
    }
    void param(const std::string& name, const std::string& type,
              int default_val, const std::string& flags = "") {
        param(name, type, ParamValue(default_val), flags);
    }
    void param(const std::string& name, const std::string& type,
              bool default_val, const std::string& flags = "") {
        param(name, type, ParamValue(default_val), flags);
    }

private:
    /**
     * Non-virtual interface for parameter initialization
     * Calls virtual config() safely
     */
    void init_params() {
        config();
    }

    // Helper to convert type string to ParamType
    static ParamType param_type_from_string(const std::string& type);

    size_t _tick_count{0};
    const ParamDef::Metadata* _metadata;
};

} // namespace PixelTheater 