#pragma once

namespace PixelTheater {

template<typename T>
class ParamRange {
public:
    ParamRange(T min, T max) : _min(min), _max(max) {}

    bool validate(T value) const {
        return value >= _min && value <= _max;
    }

    T clamp(T value) const {
        if (value < _min) return _min;
        if (value > _max) return _max;
        return value;
    }

    T min() const { return _min; }
    T max() const { return _max; }

private:
    T _min;
    T _max;
};

} // namespace PixelTheater 