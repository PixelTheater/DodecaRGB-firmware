#pragma once
#include <cstdint>
#include <cstddef>  // for size_t

namespace PixelTheater {

class Palette {
public:
    static constexpr size_t MIN_ENTRIES = 2;      // At least black and white
    static constexpr size_t MAX_ENTRIES = 256;    // 8-bit indices
    
    // Construct from raw data (4 bytes per entry: index,r,g,b)
    Palette(const uint8_t* data, size_t entries);
    
    // Access palette data
    uint8_t value_at(size_t index) const;
    bool is_valid() const { return _data != nullptr; }
    size_t size() const { return _entries; }

private:
    // Validation helpers
    bool validate_format() const;  // Check data exists and entry count
    bool validate_size() const;    // Check min/max entries
    bool validate_indices() const; // Check index ordering
    
    const uint8_t* _data;     // Raw palette data (4 bytes per entry)
    size_t _entries;          // Number of color entries
};

} // namespace PixelTheater 