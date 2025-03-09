#include "PixelTheater/params/param_schema.h"
#include "PixelTheater/scene.h"
#include <sstream>
#include <iomanip>

namespace PixelTheater {

// Helper function to escape JSON strings
std::string escape_json(const std::string& s) {
    std::ostringstream o;
    for (auto c = s.cbegin(); c != s.cend(); c++) {
        switch (*c) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if ('\x00' <= *c && *c <= '\x1f') {
                    o << "\\u"
                      << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(*c);
                } else {
                    o << *c;
                }
        }
    }
    return o.str();
}

// Convert ParameterSchema to JSON string
std::string ParameterSchema::to_json() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"name\": \"" << escape_json(name) << "\",\n";
    json << "  \"type\": \"" << escape_json(type) << "\",\n";
    json << "  \"description\": \"" << escape_json(description) << "\",\n";
    
    // Add range information if applicable
    if (type == "range" || type == "count" || 
        type == "ratio" || type == "signed_ratio" || 
        type == "angle" || type == "signed_angle") {
        json << "  \"min\": " << min_value << ",\n";
        json << "  \"max\": " << max_value << ",\n";
    }
    
    // Add default value based on type
    if (type == "range" || type == "ratio" || type == "signed_ratio" || 
        type == "angle" || type == "signed_angle") {
        json << "  \"default\": " << default_float << ",\n";
    } else if (type == "count" || type == "select") {
        json << "  \"default\": " << default_int << ",\n";
    } else if (type == "switch") {
        json << "  \"default\": " << (default_bool ? "true" : "false") << ",\n";
    }
    
    // Add options for select type
    if (type == "select" && !options.empty()) {
        json << "  \"options\": [";
        for (size_t i = 0; i < options.size(); i++) {
            json << "\"" << escape_json(options[i]) << "\"";
            if (i < options.size() - 1) {
                json << ", ";
            }
        }
        json << "],\n";
    }
    
    // Add flags
    json << "  \"flags\": \"" << escape_json(flags) << "\"\n";
    json << "}";
    
    return json.str();
}

// Convert SceneParameterSchema to JSON string
std::string SceneParameterSchema::to_json() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"name\": \"" << escape_json(scene_name) << "\",\n";
    json << "  \"description\": \"" << escape_json(scene_description) << "\",\n";
    json << "  \"parameters\": [\n";
    
    for (size_t i = 0; i < parameters.size(); i++) {
        json << "    " << parameters[i].to_json();
        if (i < parameters.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    
    json << "  ]\n";
    json << "}";
    
    return json.str();
}

// Convert schema to JSON
std::string ParamSchema::to_json(const SceneParameterSchema& schema) {
    return schema.to_json();
}

// We can't implement the template method directly in the .cpp file
// Instead, we'll provide explicit instantiations for common model types
// in a separate file or in the main application code.

} // namespace PixelTheater 