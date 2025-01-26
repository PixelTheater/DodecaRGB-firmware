#pragma once
#include "preset.h"

namespace Animation {

class PresetBuilder {
public:
    PresetBuilder(const std::string& name) : _preset(name) {}
    
    template<typename T>
    PresetBuilder& set(const std::string& name, T value) {
        if constexpr (std::is_arithmetic_v<T>) {
            _preset.values[name] = static_cast<float>(value);
        } else {
            _preset.instance_values[name] = std::make_shared<T>(value);
        }
        return *this;
    }
    
    Preset build() { return std::move(_preset); }

private:
    Preset _preset;
};

} // namespace Animation 