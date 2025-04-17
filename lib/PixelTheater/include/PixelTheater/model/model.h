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
    // REMOVED: const ModelDef& _def; // No longer need instance reference
    CRGB* _leds;  // Non-owning pointer to LED array
    std::array<Point, ModelDef::LED_COUNT> _points;
    std::array<Face, ModelDef::FACE_COUNT> _faces;

    void initialize() {
        // Initialize points
        // Access ModelDef statically
        for(size_t i = 0; i < ModelDef::LED_COUNT; ++i) {
            const auto& point_data = ModelDef::POINTS[i];
            _points[point_data.id] = Point(
                point_data.id,
                point_data.face_id,
                point_data.x,
                point_data.y,
                point_data.z
            );
        }

        // Initialize faces
        size_t led_offset = 0;
        for(size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            // Access ModelDef statically
            const auto& face_data = ModelDef::FACES[i];
            const auto& face_type = ModelDef::FACE_TYPES[face_data.type_id];
            auto& face = _faces[i];

            // Create face instance
            face = Face(
                face_type.type,
                face_data.id,
                led_offset,
                face_type.num_leds,
                _leds,
                static_cast<uint16_t>(face_type.type)  // Use enum value as number of sides
            );

            // Initialize vertices from face data
            for(size_t j = 0; j < static_cast<size_t>(face_type.type); j++) {  // Use enum value as number of sides
                face.vertices[j] = {
                    face_data.vertices[j].x,
                    face_data.vertices[j].y,
                    face_data.vertices[j].z
                };
            }

            led_offset += face_type.num_leds;
        }

        // Initialize neighbors for each point
        // Check if NEIGHBORS exists and has data before iterating
        if constexpr (sizeof(ModelDef::NEIGHBORS) > 0) {
            for(const auto& neighbor_data : ModelDef::NEIGHBORS) {
                // Ensure point_id is within bounds
                if (neighbor_data.point_id < ModelDef::LED_COUNT) {
                    // Assuming NeighborData::neighbors is a C-style array
                    // Pass pointer to the first element and the max count
                    _points[neighbor_data.point_id].setNeighbors(
                        reinterpret_cast<const Point::Neighbor*>(neighbor_data.neighbors), // Array decays to pointer, cast its type
                        ModelDef::NeighborData::MAX_NEIGHBORS // Pass the max size defined in ModelDef
                    );
                }
            }
        }
    }

public:
    // Modified constructor - only requires LED array pointer
    explicit Model(CRGB* leds)
        : _leds(leds)
    {
        initialize();
    }

    // LED array access
    struct Leds {
        CRGB* _data;  // Change to pointer
        size_t _size;
        
        CRGB& operator[](size_t i) {
            if (i >= _size) i = _size - 1;
            return _data[i];
        }
        const CRGB& operator[](size_t i) const {
            if (i >= _size) i = _size - 1;
            return _data[i];
        }

        size_t size() const { return _size; }
        
        // Just iteration support
        auto begin() { return _data; }
        auto end() { return _data + _size; }
        auto begin() const { return _data; }
        auto end() const { return _data + _size; }
    } leds{_leds, ModelDef::LED_COUNT};  // Pass size explicitly

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