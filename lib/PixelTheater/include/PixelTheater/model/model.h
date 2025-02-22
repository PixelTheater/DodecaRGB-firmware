#pragma once
#include <array>
#include "PixelTheater/model_def.h"
#include "PixelTheater/core/crgb.h"
#include "face.h"
#include "point.h"

namespace PixelTheater {

template<typename ModelDef>
class Model {
private:
    const ModelDef& _def;
    std::array<CRGB, ModelDef::LED_COUNT> _leds;
    std::array<Point, ModelDef::LED_COUNT> _points;
    std::array<Face, ModelDef::FACE_COUNT> _faces;

public:
    explicit Model(const ModelDef& def) : _def(def) {
        // Initialize points
        for(const auto& point_data : _def.POINTS) {
            _points[point_data.id] = Point(
                point_data.id,
                point_data.face_id,
                point_data.x,
                point_data.y,
                point_data.z
            );
        }

        // Initialize LEDs to black
        _leds.fill(CRGB::Black);

        // Initialize faces
        for(size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            const auto& face_data = _def.FACES[i];
            const auto& face_type = _def.FACE_TYPES[face_data.type_id];
            _faces[i] = Face(
                face_type.type,
                face_data.id,
                face_type.num_leds * i,  // LED offset
                face_type.num_leds,      // LED count
                _leds.data()            // Pointer to LED array
            );
        }
    }

    // Direct LED access - matches Model.md
    // example: model.leds[0] = CRGB::Red;
    struct Leds {
        std::array<CRGB, ModelDef::LED_COUNT>& _data;
        
        CRGB& operator[](size_t i) {
            if (i >= ModelDef::LED_COUNT) i = ModelDef::LED_COUNT - 1;
            return _data[i];
        }
        const CRGB& operator[](size_t i) const {
            if (i >= ModelDef::LED_COUNT) i = ModelDef::LED_COUNT - 1;
            return _data[i];
        }

        void fill(const CRGB& color) {
            _data.fill(color);
        }
    } leds{_leds};  // Direct member access

    // Collection access for range-based for loops
    std::array<Face, ModelDef::FACE_COUNT>& faces() { return _faces; }
    const std::array<Face, ModelDef::FACE_COUNT>& faces() const { return _faces; }

    // Points still use method access since not shown in spec
    std::array<Point, ModelDef::LED_COUNT>& points() { return _points; }
    const std::array<Point, ModelDef::LED_COUNT>& points() const { return _points; }
    
    // Size info
    static constexpr size_t led_count() { return ModelDef::LED_COUNT; }
    static constexpr size_t face_count() { return ModelDef::FACE_COUNT; }
};

} // namespace PixelTheater 