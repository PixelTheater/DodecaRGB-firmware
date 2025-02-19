#pragma once
#include <array>
#include "PixelTheater/model_def.h"
#include "PixelTheater/model/face.h"
#include "PixelTheater/model/region.h"
#include "PixelTheater/led.h"

namespace PixelTheater {

// Model provides runtime access to:
// 1. LEDs and their properties (position, neighbors)
// 2. Faces and their regions (center, rings, edges)
// 3. Model metadata (name, version, description)
template<typename ModelDef>
class Model {
public:
    // Constructor takes model definition
    explicit Model(const ModelDef& def) : _def(def) {}

    // Basic metadata access
    const char* name() const { return ModelDef::NAME; }
    const char* version() const { return ModelDef::VERSION; }
    const char* description() const { return ModelDef::DESCRIPTION; }
    
    // Collection access methods
    const std::array<Face, ModelDef::FACE_COUNT>& faces() const { return _faces; }
    const std::array<Region, ModelDef::LED_COUNT>& regions() const { return _regions; }
    const std::array<Led, ModelDef::LED_COUNT>& leds() const { return _leds; }
    
    // Indexed access
    const Face& face(size_t index) const { 
        return index < ModelDef::FACE_COUNT ? _faces[index] : _faces[0];
    }
    
    const Region& region(size_t index) const {
        return index < _region_count ? _regions[index] : _regions[0];
    }
    
    const Led& led(size_t index) const {
        return index < ModelDef::LED_COUNT ? _leds[index] : _leds[0];
    }

private:
    const ModelDef& _def;
    std::array<Face, ModelDef::FACE_COUNT> _faces;
    std::array<Region, ModelDef::LED_COUNT> _regions;  // Max possible regions
    std::array<Led, ModelDef::LED_COUNT> _leds;
    size_t _region_count{0};
};

template<typename T>
class array_view {
    const T* _data;
    size_t _size;
public:
    array_view(const T* data, size_t size) : _data(data), _size(size) {}
    const T* data() const { return _data; }
    size_t size() const { return _size; }
    const T& operator[](size_t i) const { return i < _size ? _data[i] : _data[0]; }
    const T* begin() const { return _data; }
    const T* end() const { return _data + _size; }
};

} // namespace PixelTheater 