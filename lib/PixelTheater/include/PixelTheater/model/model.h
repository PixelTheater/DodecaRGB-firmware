/**
 * @file model.h
 * @brief Defines the Model class which manages LED arrays and their geometric relationships
 * 
 * The Model class uses an efficient wrapper pattern to provide safe access to LED data:
 * 
 * 1. Data Storage:
 *    - Raw data is stored in std::arrays (_leds, _points, _faces)
 *    - Arrays are fixed-size, determined by the ModelDef template parameter
 *    - Memory layout is contiguous for optimal performance
 * 
 * 2. Access Pattern:
 *    - Wrapper structs (Leds, Points, Faces) provide controlled access to the arrays
 *    - Each wrapper holds a reference to its array, no copying or extra allocation
 *    - Wrappers implement bounds checking and iteration support
 *    - Single wrapper instance per array, created at Model construction
 * 
 * 3. Usage Example:
 *    ```cpp
 *    Model<MyModelDef> model(def);
 *    
 *    // Iterate all LEDs in the model
 *    for(auto& led : model.leds) {
 *        led = CRGB::Black;  // Clear all LEDs
 *    }
 *    
 *    // Iterate faces and their LEDs
 *    for(auto& face : model.faces) {
 *        // Set each face to a different color
 *        CRGB color = CRGB(face.id() * 50, 0, 0);
 *        for(auto& led : face.leds) {
 *            led = color;
 *        }
 *    }
 *    
 *    // Access specific LEDs through face indexing
 *    model.faces[1].leds[3] = CRGB::Blue;  // LED 3 on face 1
 *    ```
 * 
 * 4. Performance:
 *    - Zero runtime overhead - compiler can inline wrapper operations
 *    - No dynamic memory allocation after construction
 *    - Cache-friendly contiguous memory layout
 *    - Bounds checking can be disabled in release builds
 * 
 * The Model class serves as the central data structure for the LED object,
 * managing both the physical LED array and its geometric representation
 * through faces and points in 3D space.
 */

#pragma once
#include <array>
#include "PixelTheater/model_def.h"
#include "PixelTheater/core/crgb.h"
#include "PixelTheater/core/color.h"
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
        fill_solid(leds, CRGB::Black);

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

        size_t size() const { return ModelDef::LED_COUNT; }
        
        // Just iteration support
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