#pragma once
#include <array>
#include "PixelTheater/core/array_view.h"
#include "PixelTheater/model_def.h"
#include "face.h"
#include "region.h"
#include "led.h"
#include "point.h"

namespace PixelTheater {

// Model provides runtime access to:
// 1. LEDs and their properties (position, neighbors)
// 2. Faces and their regions (center, rings, edges)
// 3. Model metadata (name, version, description)
template<typename ModelDef>
class Model {
public:
    // Constructor takes model definition
    explicit Model(const ModelDef& def) : _def(def) {
        // Initialize points first
        for(const auto& point_data : ModelDef::POINTS) {
            _points[point_data.id] = Point(
                point_data.id,
                point_data.face_id,
                point_data.x,
                point_data.y,
                point_data.z
            );
        }

        // Initialize LEDs with points
        for(size_t i = 0; i < ModelDef::LED_COUNT; i++) {
            _leds[i] = Led(_colors[i], _points[i], i);
        }

        // Initialize faces with their types
        for(size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            const auto& face_data = ModelDef::FACES[i];
            const auto& face_type = ModelDef::FACE_TYPES[face_data.type_id];
            _faces[i] = Face(face_type.type, face_data.id);
        }

        // Initialize regions and link to faces
        for(const auto& region_data : ModelDef::REGIONS) {
            // Create region with correct type
            auto& region = _regions[_region_count++];
            region = Region(region_data.type);

            // Get LED count from region data
            size_t led_count = region_data.led_count;
            
#ifdef UNIT_TEST
            printf("Region %d type=%d led_count=%zu\n", 
                   region_data.id, 
                   static_cast<int>(region_data.type),
                   led_count);
#endif

            // Set up LED span with actual count
            uint16_t* indices = new uint16_t[led_count];  // Allocate new array
            for(size_t i = 0; i < led_count; i++) {
                indices[i] = region_data.led_ids[i];
            }
            region._leds = LedSpan(_leds.data(), indices, led_count);

            // Link to face
            auto& face = _faces[region_data.face_id];
            switch(region_data.type) {
                case RegionType::Center:
                    face._center = region;
                    break;
                case RegionType::Ring:
                    face._rings[face._ring_count++] = region;
                    break;
                case RegionType::Edge:
                    face._edges[face._edge_count++] = region;
                    break;
                default:
                    break;
            }
            face._regions[face._region_count++] = region;
        }
    }

    // Basic metadata access
    const char* name() const { return ModelDef::NAME; }
    const char* version() const { return ModelDef::VERSION; }
    const char* description() const { return ModelDef::DESCRIPTION; }
    
    // Collection access methods
    array_view<const Face> faces() const { 
        return array_view<const Face>(_faces.data(), ModelDef::FACE_COUNT); 
    }
    array_view<const Region> regions() const { return array_view<const Region>(_regions.data(), _region_count); }
    array_view<const Led> leds() const { return array_view<const Led>(_leds.data(), _leds.size()); }
    
    // Indexed access (operator[])
    const Face& operator[](size_t index) const {
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
    std::array<Point, ModelDef::LED_COUNT> _points;
    std::array<CRGB, ModelDef::LED_COUNT> _colors;
    size_t _region_count{0};
};

} // namespace PixelTheater 