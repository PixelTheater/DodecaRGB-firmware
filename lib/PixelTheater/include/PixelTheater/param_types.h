#pragma once
#include "param_range.h"
#include <map>

namespace PixelTheater {

// Standard parameter types with predefined ranges
class Ratio : public ParamRange<float> {
public:
    static constexpr float DEFAULT = 0.0f;
    Ratio() : ParamRange<float>(0.0f, 1.0f) {}
};

class SignedRatio : public ParamRange<float> {
public:
    static constexpr float DEFAULT = 0.0f;
    SignedRatio() : ParamRange<float>(-1.0f, 1.0f) {}
};

class Angle : public ParamRange<float> {
public:
    static constexpr float PI = 3.14159265359f;
    static constexpr float DEFAULT = 0.0f;
    Angle() : ParamRange<float>(0.0f, PI) {}
};

class SignedAngle : public ParamRange<float> {
public:
    static constexpr float PI = 3.14159265359f;
    static constexpr float DEFAULT = 0.0f;
    SignedAngle() : ParamRange<float>(-PI, PI) {}
};

class Count : public ParamRange<int> {
public:
    static constexpr int DEFAULT = 0;
    Count(int max = 100) : ParamRange<int>(0, max) {}
};

template<typename T>
class Range : public ParamRange<T> {
public:
    Range(T min, T max) : ParamRange<T>(min, max) {}
};

class Switch : public ParamRange<bool> {
public:
    static constexpr bool DEFAULT = false;
    Switch() : ParamRange<bool>(false, true) {}
};

class Select : public ParamRange<int> {
public:
    Select(int max_value) : ParamRange<int>(0, max_value) {}
    
    // Helper to validate a named value exists
    bool has_value(const std::string& name) const {
        return _values.find(name) != _values.end();
    }
    
    // Get integer value for a name
    int value_of(const std::string& name) const {
        auto it = _values.find(name);
        return it != _values.end() ? it->second : 0;
    }
    
    // Add a named value mapping
    void add_value(const std::string& name, int value) {
        _values[name] = value;
    }

private:
    std::map<std::string, int> _values;
};

} // namespace PixelTheater 