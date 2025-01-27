#pragma once

namespace PixelTheater {

enum ParamFlags {
    None = 0,
    Clamp = 1 << 0,  // Limit values to range
    Wrap = 1 << 1,   // Wrap around range bounds
    Slew = 1 << 2,   // Smooth transitions (TODO)
};

template<typename T>
class ParamRange {
public:
    ParamRange(T min, T max) : _min(min), _max(max) {}

    bool validate(T value) const {
        return value >= _min && value <= _max;
    }

    T apply(T value, ParamFlags flags = Clamp) const {
        if (flags & Clamp) {
            if (value < _min) return _min;
            if (value > _max) return _max;
        }
        // TODO: Wrap behavior
        return value;
    }

    T min() const { return _min; }
    T max() const { return _max; }

private:
    T _min;
    T _max;
};

} // namespace PixelTheater 