#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "param_def.h"
#include "param_value.h"
// #include "PixelTheater/scene.h" // REMOVED Include full definition

namespace PixelTheater {

// Forward declarations
class Scene; // ADDED back forward declaration
// template<typename ModelDef> class Scene;  // Only declare the template version

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
    static ParameterSchema from_param_def(const ParamDef& def);
    
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
    // Function to generate the schema from a Scene instance
    // DECLARATION ONLY
    SceneParameterSchema generate_schema(const Scene& scene);
    
    // Convert schema to JSON
    std::string to_json(const SceneParameterSchema& schema);
}

} // namespace PixelTheater 