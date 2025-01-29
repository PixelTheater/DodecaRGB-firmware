#pragma once
#include <cstdint>
#include <cstddef>  // for size_t

namespace PixelTheater {

class Palette {
public:
    // Create from gradient data
    Palette(const uint8_t* data, size_t entries);
    
    // Get raw value at index for testing
    uint8_t value_at(size_t index) const;
    
    // Get number of entries
    size_t size() const { return _entries; }
    
    bool is_valid() const { return _data != nullptr; }

private:
    // Validation helpers
    bool validate_size() const;      // Check entry count 2-16
    bool validate_indices() const;   // Check index values 0-255 ascending
    bool validate_format() const;    // Check overall data format

    static constexpr size_t MIN_ENTRIES = 2;
    static constexpr size_t MAX_ENTRIES = 16;

    const uint8_t* _data;  // Format: index,r,g,b repeating
    size_t _entries;       // Number of 4-byte entries
};

} // namespace PixelTheater 