#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "param_def.h"
#include "param_value.h"

namespace PixelTheater {

// Forward declarations
template<typename ModelDef> class Scene;  // Only declare the template version

/**
 * Represents a single parameter's schema for serialization
 */
struct ParameterSchema {
    std::string name;
    std::string type;
    std::string description;
    
    // Range information
    float min_value;
    float max_value;
    
    // Default values (only one is used based on type)
    float default_float;
    int default_int;
    bool default_bool;
    
    // Options for select type
    std::vector<std::string> options;
    
    // Flags
    std::string flags;
    
    // Create from ParamDef
    static ParameterSchema from_param_def(const ParamDef& def) {
        ParameterSchema schema;
        schema.name = def.name;
        schema.type = ParamHandlers::TypeHandler::get_name(def.type);
        schema.description = def.description;
        schema.min_value = def.min_value;
        schema.max_value = def.max_value;
        schema.default_float = def.default_float;
        schema.default_int = def.default_int;
        schema.default_bool = def.default_bool;
        schema.options = def.options;
        schema.flags = ParamHandlers::FlagHandler::to_string(def.flags);
        return schema;
    }
    
    // Convert to JSON string
    std::string to_json() const;
};

/**
 * Represents a complete schema for all parameters in a scene
 */
struct SceneParameterSchema {
    std::string scene_name;
    std::string scene_description;
    std::vector<ParameterSchema> parameters;
    
    // Convert to JSON string
    std::string to_json() const;
};

/**
 * Helper functions for parameter schema generation
 */
namespace ParamSchema {
    // Generate schema for a scene
    template<typename ModelDef>
    SceneParameterSchema generate_schema(const Scene<ModelDef>& scene) {
        SceneParameterSchema schema;
        schema.scene_name = scene.name();
        schema.scene_description = scene.description();
        
        // Get all parameter names
        auto param_names = scene.get_parameter_names();
        
        // Sort parameter names for consistent ordering
        std::sort(param_names.begin(), param_names.end());
        
        // Add each parameter to the schema
        for (const auto& name : param_names) {
            const ParamDef& param_def = scene.get_parameter_metadata(name);
            schema.parameters.push_back(ParameterSchema::from_param_def(param_def));
        }
        
        return schema;
    }
    
    // Convert schema to JSON
    std::string to_json(const SceneParameterSchema& schema);
}

} // namespace PixelTheater 