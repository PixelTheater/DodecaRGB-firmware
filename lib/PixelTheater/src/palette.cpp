#include "PixelTheater/palette.h"

namespace PixelTheater {

Palette::Palette(const uint8_t* data, size_t entries) 
    : _data(data)
    , _entries(entries/4)  // Convert bytes to number of entries
{
    // Validate data
    if (!validate_format() || !validate_size() || !validate_indices()) {
        _data = nullptr;
        _entries = 0;
    }
}

uint8_t Palette::value_at(size_t index) const {
    if (!is_valid() || index >= (_entries * 4)) {
        return 0;
    }
    return _data[index];
}

bool Palette::validate_format() const {
    return _data != nullptr && _entries > 0;
}

bool Palette::validate_size() const {
    return _entries >= MIN_ENTRIES && _entries <= MAX_ENTRIES;
}

bool Palette::validate_indices() const {
    if (!validate_format()) return false;

    // First index must be 0
    if (_data[0] != 0) return false;

    // Last index must be 255
    if (_data[(_entries-1) * 4] != 255) return false;

    // Check ascending order
    for (size_t i = 1; i < _entries; i++) {
        uint8_t prev = _data[(i-1) * 4];
        uint8_t curr = _data[i * 4];
        if (curr <= prev) return false;
    }

    return true;
}

} // namespace PixelTheater 