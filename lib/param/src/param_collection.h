#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "param.h"
#include "param_builder.h"
#include <regex>

class ParameterCollection {
public:
    // Fluent interface for defining parameters
    ParamBuilder& param(const std::string& name) {
        // Check if we're already built
        if (_is_built) {
            throw std::runtime_error("Cannot add parameters after build()");
        }
        
        // Validate parameter name
        static const std::regex name_pattern("^[a-zA-Z_][a-zA-Z0-9_]*$");
        if (!std::regex_match(name, name_pattern)) {
            throw std::invalid_argument(
                "Invalid parameter name '" + name + "'. " +
                "Names must start with letter/underscore and contain only letters, numbers, and underscores."
            );
        }
        
        // Check for duplicate names
        for (const auto& builder : _builders) {
            if (builder->getName() == name) {
                throw std::invalid_argument("Parameter '" + name + "' already exists");
            }
        }
        
        // Create new builder and store it
        _builders.push_back(std::make_unique<ParamBuilder>(name));
        return *_builders.back();
    }

    // Get a parameter definition by name
    const ParamDefinition* get(const std::string& name) const {
        auto it = _params.find(name);
        return it != _params.end() ? &it->second : nullptr;
    }

    // Build all parameters (called after all params are defined)
    void build() {
        _params.clear();
        for (const auto& builder : _builders) {
            auto param = builder->build();
            _params.emplace(param.name, std::move(param));
        }
        _builders.clear();
        _is_built = true;
    }

private:
    std::vector<std::unique_ptr<ParamBuilder>> _builders;
    std::unordered_map<std::string, ParamDefinition> _params;
    bool _is_built = false;
}; 