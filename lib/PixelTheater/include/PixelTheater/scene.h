#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include "settings.h"
#include "settings_proxy.h"
#include "params/param_def.h"
#include "params/param_value.h"
#include "model/model.h"
#include "stage.h"

// Forward declare to avoid circular dependency
namespace PixelTheater {
    struct SceneParameterSchema;
    namespace ParamSchema {
        template<typename ModelDef>
        SceneParameterSchema generate_schema(const Scene<ModelDef>& scene);
    }
}

// Now include the param_schema.h file
#include "params/param_schema.h"

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
          const ParamDef* metadata = nullptr)
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
        settings.reset_all();
    }

    /**
     * Get scene name
     * @return Name of the scene
     */
    const char* name() const {
        return _metadata ? _metadata->name.c_str() : "Unnamed Scene";
    }

    /**
     * Get scene description
     * @return Description of the scene
     */
    const char* description() const {
        return _metadata ? _metadata->description.c_str() : "";
    }

    /**
     * Get number of ticks processed
     * @return Tick count
     */
    size_t tick_count() const { return _tick_count; }

    /**
     * Get all parameter names defined for this scene
     * @return Vector of parameter names
     */
    std::vector<std::string> get_parameter_names() const {
        return _settings_storage.get_parameter_names();
    }

    /**
     * Get metadata for a specific parameter
     * @param name Parameter name
     * @return Parameter metadata
     */
    const ParamDef& get_parameter_metadata(const std::string& name) const {
        return _settings_storage.get_metadata(name);
    }

    /**
     * Check if a parameter exists
     * @param name Parameter name
     * @return True if the parameter exists, false otherwise
     */
    bool has_parameter(const std::string& name) const {
        return _settings_storage.has_parameter(name);
    }

    /**
     * Get the type of a parameter
     * @param name Parameter name
     * @return Parameter type
     */
    ParamType get_parameter_type(const std::string& name) const {
        return _settings_storage.get_type(name);
    }

    /**
     * Get parameter schema for this scene
     * @return Parameter schema
     */
    SceneParameterSchema parameter_schema() const {
        return ParamSchema::generate_schema(*this);
    }

    /**
     * Get parameter schema as JSON string
     * @return JSON string
     */
    std::string parameter_schema_json() const {
        return parameter_schema().to_json();
    }

    Stage<ModelDef>& stage;
    Settings _settings_storage;
    SettingsProxy settings;      // Now public - main interface for parameter access

protected:
    /**
     * Define a parameter with a string type and default value
     * @param name Parameter name
     * @param type Parameter type as string (e.g., "ratio", "count", "switch")
     * @param default_val Default value
     * @param flags Optional flags (e.g., "clamp", "wrap")
     * @param description Optional description
     */
    void param(const std::string& name, const std::string& type,
              const ParamValue& default_val, const std::string& flags = "",
              const std::string& description = "") {
        _settings_storage.add_parameter_from_strings(name, type, default_val, flags, description);
    }

    /**
     * Configure scene parameters
     * Called during initialization if no generated params
     */
    virtual void config() {}

    /**
     * Define a parameter with a float default value
     */
    void param(const std::string& name, const std::string& type,
              float default_val, const std::string& flags = "",
              const std::string& description = "") {
        param(name, type, ParamValue(default_val), flags, description);
    }
    
    /**
     * Define a parameter with an int default value
     */
    void param(const std::string& name, const std::string& type,
              int default_val, const std::string& flags = "",
              const std::string& description = "") {
        param(name, type, ParamValue(default_val), flags, description);
    }
    
    /**
     * Define a parameter with a bool default value
     */
    void param(const std::string& name, const std::string& type,
              bool default_val, const std::string& flags = "",
              const std::string& description = "") {
        param(name, type, ParamValue(default_val), flags, description);
    }

    /**
     * Define a parameter with an integer range
     * @param name Parameter name
     * @param type Parameter type (usually "count")
     * @param min Minimum value
     * @param max Maximum value
     * @param default_val Default value
     * @param flags Optional flags
     * @param description Optional description
     */
    void param(const std::string& name, const std::string& type,
              int min, int max, int default_val, const std::string& flags = "",
              const std::string& description = "") {
        if (type == "count") {
            _settings_storage.add_count_parameter(name, min, max, default_val, flags, description);
        } else {
            param(name, type, default_val, flags, description);
        }
    }
    
    /**
     * Define a parameter with a float range
     * @param name Parameter name
     * @param type Parameter type (usually "range")
     * @param min Minimum value
     * @param max Maximum value
     * @param default_val Default value
     * @param flags Optional flags
     * @param description Optional description
     */
    void param(const std::string& name, const std::string& type,
              float min, float max, float default_val, const std::string& flags = "",
              const std::string& description = "") {
        if (type == "range") {
            _settings_storage.add_range_parameter(name, min, max, default_val, flags, description);
        } else {
            param(name, type, default_val, flags, description);
        }
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
    static ParamType param_type_from_string(const std::string& type) {
        if (type == "ratio") return ParamType::ratio;
        if (type == "signed_ratio") return ParamType::signed_ratio;
        if (type == "angle") return ParamType::angle;
        if (type == "signed_angle") return ParamType::signed_angle;
        if (type == "range") return ParamType::range;
        if (type == "count") return ParamType::count;
        if (type == "switch") return ParamType::switch_type;
        if (type == "select") return ParamType::select;
        // Default to range if unknown
        return ParamType::range;
    }

    size_t _tick_count{0};
    const ParamDef* _metadata;
};

} // namespace PixelTheater 