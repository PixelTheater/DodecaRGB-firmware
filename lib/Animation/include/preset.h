#pragma once

#include <string>
#include <unordered_map>
#include <memory>

namespace Animation {

// Store preset values
struct Preset {
    std::string name;
    std::unordered_map<std::string, float> values;
    std::unordered_map<std::string, std::shared_ptr<void>> instance_values;

    Preset() = default;  // Add default constructor

    // Constructor with name
    explicit Preset(const std::string& name) : name(name) {}
};

} // namespace Animation
