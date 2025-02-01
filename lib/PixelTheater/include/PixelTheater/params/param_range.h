#pragma once
#include <map>
#include <string>

namespace PixelTheater {

// Standard ranges for semantic types
namespace Constants {
    constexpr float RATIO_MIN = 0.0f;
    constexpr float RATIO_MAX = 1.0f;
    constexpr float PI = 3.14159265359f;
    
    constexpr float SIGNED_RATIO_MIN = -1.0f;
    constexpr float SIGNED_RATIO_MAX = 1.0f;
    
    constexpr float ANGLE_MIN = 0.0f;
    constexpr float ANGLE_MAX = PI;
    
    constexpr float SIGNED_ANGLE_MIN = -PI;
    constexpr float SIGNED_ANGLE_MAX = PI;
}

// Base range validation
template<typename T>
class ParamRange {
public:
    ParamRange(T min, T max) : _min(min), _max(max) {}

    bool validate(T value) const {
        return value >= _min && value <= _max;
    }

    T min() const { return _min; }
    T max() const { return _max; }

protected:
    T _min;
    T _max;
};

// Standard parameter types with predefined ranges
class Ratio : public ParamRange<float> {
public:
    static constexpr float DEFAULT = 0.0f;
    Ratio() : ParamRange<float>(Constants::RATIO_MIN, Constants::RATIO_MAX) {}
};

class SignedRatio : public ParamRange<float> {
public:
    static constexpr float DEFAULT = 0.0f;
    SignedRatio() : ParamRange<float>(Constants::SIGNED_RATIO_MIN, Constants::SIGNED_RATIO_MAX) {}
};

class Angle : public ParamRange<float> {
public:
    static constexpr float DEFAULT = 0.0f;
    Angle() : ParamRange<float>(Constants::ANGLE_MIN, Constants::ANGLE_MAX) {}
};

class SignedAngle : public ParamRange<float> {
public:
    static constexpr float DEFAULT = 0.0f;
    SignedAngle() : ParamRange<float>(Constants::SIGNED_ANGLE_MIN, Constants::SIGNED_ANGLE_MAX) {}
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

// Add Switch type
class Switch : public ParamRange<bool> {
public:
    static constexpr bool DEFAULT = false;
    Switch() : ParamRange<bool>(false, true) {}
};

} // namespace PixelTheater 