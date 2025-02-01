#include "PixelTheater/settings.h"
#include <iostream>

namespace PixelTheater {

Settings::Settings(const ParamDef* params, size_t count) {
    for (size_t i = 0; i < count; i++) {
        const auto& def = params[i];
        _metadata[def.name] = def;
        
        switch (def.type) {
            case ParamType::switch_type:
                _params[def.name] = std::make_unique<Parameter<bool>>(
                    def.name, false, true, def.bool_default, def.flags);
                break;

            case ParamType::range:
                _params[def.name] = std::make_unique<Parameter<float>>(
                    def.name, def.range_min, def.range_max, def.default_val, def.flags);
                break;

            case ParamType::count:
                _params[def.name] = std::make_unique<Parameter<int>>(
                    def.name, def.range_min_i, def.range_max_i, def.default_val_i, def.flags);
                break;

            case ParamType::ratio:
                _params[def.name] = std::make_unique<Parameter<float>>(
                    def.name, 0.0f, 1.0f, def.float_default, def.flags);
                break;

            case ParamType::signed_ratio:
                _params[def.name] = std::make_unique<Parameter<float>>(
                    def.name, -1.0f, 1.0f, def.float_default, def.flags);
                break;

            case ParamType::angle:
                _params[def.name] = std::make_unique<Parameter<float>>(
                    def.name, 0.0f, Constants::PI, def.float_default, def.flags);
                break;

            case ParamType::signed_angle:
                _params[def.name] = std::make_unique<Parameter<float>>(
                    def.name, -Constants::PI, Constants::PI, def.float_default, def.flags);
                break;

            case ParamType::select: {
                size_t count = 0;
                while (def.options[count]) count++;
                _params[def.name] = std::make_unique<Parameter<int>>(
                    def.name, 0, count-1, def.default_idx, def.flags);
                break;
            }

            case ParamType::palette:
            case ParamType::bitmap:
                // TODO: Handle resource types
                break;
        }
    }
}

SettingsProxy Settings::operator[](const std::string& name) {
    if (_params.find(name) == _params.end()) {
        throw std::out_of_range("Parameter not found: " + name);
    }
    return SettingsProxy(*this, name);
}

const SettingsProxy Settings::operator[](const std::string& name) const {
    if (_params.find(name) == _params.end()) {
        throw std::out_of_range("Parameter not found: " + name);
    }
    return SettingsProxy(const_cast<Settings&>(*this), name);
}

// Type-specific getters
float Settings::get_float(const std::string& name) const {
    auto it = _params.find(name);
    if (it == _params.end()) {
        throw std::out_of_range("Parameter not found: " + name);
    }
    auto* param = dynamic_cast<const Parameter<float>*>(it->second.get());
    if (!param) {
        throw std::bad_cast();
    }
    return param->get();
}

int Settings::get_int(const std::string& name) const {
    auto it = _params.find(name);
    if (it == _params.end()) {
        throw std::out_of_range("Parameter not found: " + name);
    }
    auto* param = dynamic_cast<const Parameter<int>*>(it->second.get());
    if (!param) {
        throw std::bad_cast();
    }
    return param->get();
}

bool Settings::get_bool(const std::string& name) const {
    auto it = _params.find(name);
    if (it == _params.end()) {
        throw std::out_of_range("Parameter not found: " + name);
    }
    auto* param = dynamic_cast<const Parameter<bool>*>(it->second.get());
    if (!param) {
        throw std::bad_cast();
    }
    return param->get();
}

// Type-specific setters
void Settings::set_float(const std::string& name, float value) {
    auto it = _params.find(name);
    if (it == _params.end()) {
        throw std::out_of_range("Parameter not found: " + name);
    }
    
    auto* param = dynamic_cast<Parameter<float>*>(it->second.get());
    if (!param) {
        throw std::bad_cast();
    }
    
    if (!param->set(value)) {
        throw std::out_of_range("Value out of range for parameter: " + name);
    }
}

void Settings::set_int(const std::string& name, int value) {
    auto it = _params.find(name);
    if (it == _params.end()) {
        throw std::out_of_range("Parameter not found: " + name);
    }
    
    auto* param = dynamic_cast<Parameter<int>*>(it->second.get());
    if (!param) {
        throw std::bad_cast();
    }
    
    if (!param->set(value)) {
        throw std::out_of_range("Value out of range for parameter: " + name);
    }
}

void Settings::set_bool(const std::string& name, bool value) {
    auto it = _params.find(name);
    if (it == _params.end()) {
        throw std::out_of_range("Parameter not found: " + name);
    }
    
    auto* param = dynamic_cast<Parameter<bool>*>(it->second.get());
    if (!param) {
        throw std::bad_cast();
    }
    
    if (!param->set(value)) {
        throw std::out_of_range("Value out of range for parameter: " + name);
    }
}

const ParamDef& Settings::get_metadata(const std::string& name) const {
    auto it = _metadata.find(name);
    if (it == _metadata.end()) throw std::out_of_range(name);
    return it->second;
}

void Settings::reset_all() {
    for (auto& [name, param] : _params) {
        param->reset();
    }
}

} // namespace PixelTheater 