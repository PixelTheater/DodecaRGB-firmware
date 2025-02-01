#include "PixelTheater/settings_proxy.h"
#include "PixelTheater/settings.h"
#include <iostream>

namespace PixelTheater {

SettingsProxy::SettingsProxy(Settings& settings, const std::string& name)
    : _settings(settings)
    , _name(name)
{}

// Value access
SettingsProxy::operator float() const {
    try {
        return _settings.get_float(_name);
    } catch (const std::exception&) {
        throw std::bad_cast();
    }
}

SettingsProxy::operator int() const {
    try {
        return _settings.get_int(_name);
    } catch (const std::exception&) {
        throw std::bad_cast();
    }
}

SettingsProxy::operator bool() const {
    try {
        return _settings.get_bool(_name);
    } catch (const std::exception&) {
        throw std::bad_cast();
    }
}

// Assignment
SettingsProxy& SettingsProxy::operator=(float value) {
    try {
        _settings.set_float(_name, value);
        return *this;
    } catch (const std::out_of_range&) {
        throw;  // Re-throw the exception
    }
}

SettingsProxy& SettingsProxy::operator=(int value) {
    try {
        _settings.set_int(_name, value);
        return *this;
    } catch (const std::out_of_range&) {
        throw;  // Re-throw the exception
    }
}

SettingsProxy& SettingsProxy::operator=(bool value) {
    try {
        _settings.set_bool(_name, value);
        return *this;
    } catch (const std::out_of_range&) {
        throw;  // Re-throw the exception
    }
}

// Parameter interface
float SettingsProxy::max() const {
    const auto& meta = metadata();
    switch (meta.type) {
        case ParamType::range:
            return meta.range_max;
        case ParamType::ratio:
            return 1.0f;
        case ParamType::signed_ratio:
            return 1.0f;
        case ParamType::angle:
            return Constants::PI;
        case ParamType::signed_angle:
            return Constants::PI;
        default:
            throw std::runtime_error("Parameter has no max value");
    }
}

float SettingsProxy::min() const {
    const auto& meta = metadata();
    switch (meta.type) {
        case ParamType::range:
            return meta.range_min;
        case ParamType::ratio:
            return 0.0f;
        case ParamType::signed_ratio:
            return -1.0f;
        case ParamType::angle:
            return 0.0f;
        case ParamType::signed_angle:
            return -Constants::PI;
        default:
            throw std::runtime_error("Parameter has no min value");
    }
}

float SettingsProxy::default_value() const {
    const auto& meta = metadata();
    switch (meta.type) {
        case ParamType::range:
            return meta.default_val;
        case ParamType::ratio:
        case ParamType::angle:
            return meta.float_default;
        default:
            throw std::runtime_error("Parameter has no float default");
    }
}

const char* SettingsProxy::description() const {
    return metadata().description;
}

const ParamDef& SettingsProxy::metadata() const {
    return _settings.get_metadata(_name);
}

} // namespace PixelTheater 