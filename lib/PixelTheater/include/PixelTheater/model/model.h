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
        size_t led_offset = 0;  // Start at beginning of LED strip
        for(size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            const auto& face_data = _def.FACES[i];
            const auto& face_type = _def.FACE_TYPES[face_data.type_id];
            _faces[i] = Face(
                face_type.type,
                face_data.id,
                led_offset,             // Current offset in LED strip
                face_type.num_leds,     // Number of LEDs in this face
                _leds.data()           // Pointer to start of LED strip
            );
            led_offset += face_type.num_leds;  // Move offset to start of next face
        }
    }

    // LED array access
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

        // Add size() method
        size_t size() const { return ModelDef::LED_COUNT; }

        // Allow iteration
        auto begin() { return _data.begin(); }
        auto end() { return _data.end(); }
        auto begin() const { return _data.begin(); }
        auto end() const { return _data.end(); }
    } leds{_leds};

    // Point array access
    struct Points {
        std::array<Point, ModelDef::LED_COUNT>& _data;
        
        Point& operator[](size_t i) {
            if (i >= ModelDef::LED_COUNT) i = ModelDef::LED_COUNT - 1;
            return _data[i];
        }
        const Point& operator[](size_t i) const {
            if (i >= ModelDef::LED_COUNT) i = ModelDef::LED_COUNT - 1;
            return _data[i];
        }

        // Add size() method
        size_t size() const { return ModelDef::LED_COUNT; }

        // Allow iteration
        auto begin() { return _data.begin(); }
        auto end() { return _data.end(); }
        auto begin() const { return _data.begin(); }
        auto end() const { return _data.end(); }
    } points{_points};

    // Face array access
    struct Faces {
        std::array<Face, ModelDef::FACE_COUNT>& _data;
        
        Face& operator[](size_t i) {
            if (i >= ModelDef::FACE_COUNT) i = ModelDef::FACE_COUNT - 1;
            return _data[i];
        }
        const Face& operator[](size_t i) const {
            if (i >= ModelDef::FACE_COUNT) i = ModelDef::FACE_COUNT - 1;
            return _data[i];
        }

        // Add size() method
        size_t size() const { return ModelDef::FACE_COUNT; }

        // Allow iteration
        auto begin() { return _data.begin(); }
        auto end() { return _data.end(); }
        auto begin() const { return _data.begin(); }
        auto end() const { return _data.end(); }
    } faces{_faces};

    // Size info
    static constexpr size_t led_count() { return ModelDef::LED_COUNT; }
    static constexpr size_t face_count() { return ModelDef::FACE_COUNT; }
};

} // namespace PixelTheater 