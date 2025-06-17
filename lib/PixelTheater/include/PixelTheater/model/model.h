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
#include <memory>
#include <vector>
#include <cmath>
#include "PixelTheater/model_def.h"
#include "PixelTheater/core/crgb.h"
#include "PixelTheater/core/color.h"
#include "PixelTheater/core/imodel.h"
#include "face.h"
#include "point.h"

namespace PixelTheater {

template<typename ModelDef>
class Model : public IModel {
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
        // CRITICAL FIX: Calculate LED offsets based on original face ID order (physical wiring),
        // not array position order (which can be remapped)
        for(size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            // Access ModelDef statically
            const auto& face_data = ModelDef::FACES[i];
            const auto& face_type = ModelDef::FACE_TYPES[face_data.type_id];
            auto& face = _faces[i];

            // Calculate LED offset based on original face ID (physical wiring order)
            size_t led_offset = 0;
            for (size_t f = 0; f < face_data.id; f++) {
                // Find face with original ID = f and get its LED count
                for (size_t j = 0; j < ModelDef::FACE_COUNT; j++) {
                    if (ModelDef::FACES[j].id == f) {
                        const auto& prev_face_type = ModelDef::FACE_TYPES[ModelDef::FACES[j].type_id];
                        led_offset += prev_face_type.num_leds;
                        break;
                    }
                }
            }

            // Create face instance with correct LED offset
            face = Face(
                face_type.type,
                face_data.id,
                led_offset,  // Now correctly based on original face ID order
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
        const Model* _model;
        
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
    } faces{_faces, this};

    // LED Group access
    struct LedGroup {
        const char* name;
        uint8_t face_type_id;
        uint8_t led_count;
        const uint16_t* led_indices;
        CRGB* face_leds;  // Pointer to the face's LED array
        
        // Iterator support for group LEDs
        struct Iterator {
            const uint16_t* indices;
            CRGB* face_leds;
            uint8_t current;
            uint8_t count;
            
            Iterator(const uint16_t* idx, CRGB* leds, uint8_t curr, uint8_t cnt)
                : indices(idx), face_leds(leds), current(curr), count(cnt) {}
            
            CRGB& operator*() { return face_leds[indices[current]]; }
            Iterator& operator++() { current++; return *this; }
            bool operator!=(const Iterator& other) const { return current != other.current; }
        };
        
        Iterator begin() { return Iterator(led_indices, face_leds, 0, led_count); }
        Iterator end() { return Iterator(led_indices, face_leds, led_count, led_count); }
        
        // Access individual LEDs in the group
        CRGB& operator[](size_t i) {
            if (i >= led_count) i = led_count - 1;
            return face_leds[led_indices[i]];
        }
        
        size_t size() const { return led_count; }
    };
    

    
    // Internal helper: Find LED group by name for a specific face (used by FaceProxy)
    LedGroup group(const char* name, uint8_t face_id) const {
        if (face_id >= ModelDef::FACE_COUNT) {
            return LedGroup{"", 0, 0, nullptr, nullptr};
        }
        
        const auto& face_data = ModelDef::FACES[face_id];
        
        // Search for matching group in the face's type (handle empty arrays gracefully)
        for (size_t i = 0; i < ModelDef::LED_GROUPS.size(); i++) {
            const auto& group_data = ModelDef::LED_GROUPS[i];
            
            // Check if this group belongs to the face's type
            if (group_data.face_type_id != face_data.type_id) {
                continue;
            }
            
            // Compare names
            bool name_matches = true;
            for (size_t j = 0; j < 16; j++) {
                if (group_data.name[j] != name[j]) {
                    name_matches = false;
                    break;
                }
                if (group_data.name[j] == '\0' && name[j] == '\0') {
                    break;
                }
            }
            
            if (name_matches) {
                // Calculate LED offset for this specific face based on original face ID order
                size_t led_offset = 0;
                for (size_t f = 0; f < face_id; f++) {
                    // Find face with original ID = f and get its LED count
                    for (size_t j = 0; j < ModelDef::FACE_COUNT; j++) {
                        if (ModelDef::FACES[j].id == f) {
                            const auto& prev_face_type = ModelDef::FACE_TYPES[ModelDef::FACES[j].type_id];
                    led_offset += prev_face_type.num_leds;
                            break;
                        }
                    }
                }
                
                return LedGroup{
                    group_data.name,
                    group_data.face_type_id,
                    group_data.led_count,
                    group_data.led_indices,
                    _leds + led_offset
                };
            }
        }
        
        return LedGroup{"", 0, 0, nullptr, nullptr};
    }
    
    // Edge access
    struct Edge {
        uint8_t face_id;
        uint8_t edge_index;
        struct Point3D {
            float x, y, z;
        };
        Point3D start_vertex;
        Point3D end_vertex;
        int8_t connected_face_id;  // -1 if no connection
        
        bool has_connection() const { return connected_face_id != -1; }
    };
    
    // Access edge by index
    Edge edges(size_t index) const {
        if (index >= ModelDef::EDGES.size()) {
            // Return invalid edge
            return Edge{0, 0, {0, 0, 0}, {0, 0, 0}, -1};
        }
        
        const auto& edge_data = ModelDef::EDGES[index];
        return Edge{
            edge_data.face_id,
            edge_data.edge_index,
            {edge_data.start_vertex.x, edge_data.start_vertex.y, edge_data.start_vertex.z},
            {edge_data.end_vertex.x, edge_data.end_vertex.y, edge_data.end_vertex.z},
            static_cast<int8_t>(edge_data.connected_face_id)
        };
    }
    
    // === Internal helper methods for FaceProxy ===
    
    // === Internal helper methods for FaceProxy ===
    
    // Get all edges for a specific face
    struct FaceEdges {
        const Model* model;
        uint8_t face_id;
        
        struct Iterator {
            const Model* model;
            uint8_t target_face_id;
            size_t current_edge;
            size_t max_edges;
            
            Iterator(const Model* m, uint8_t fid, size_t start) 
                : model(m), target_face_id(fid), current_edge(start), max_edges(ModelDef::EDGES.size()) {
                // Skip to first edge for this face
                while (current_edge < max_edges && 
                       ModelDef::EDGES[current_edge].face_id != target_face_id) {
                    current_edge++;
                }
            }
            
            Edge operator*() const { return model->edges(current_edge); }
            Iterator& operator++() { 
                current_edge++;
                // Skip to next edge for this face
                while (current_edge < max_edges && 
                       ModelDef::EDGES[current_edge].face_id != target_face_id) {
                    current_edge++;
                }
                return *this; 
            }
            bool operator!=(const Iterator& other) const { 
                return current_edge != other.current_edge; 
            }
        };
        
        Iterator begin() const { return Iterator(model, face_id, 0); }
        Iterator end() const { return Iterator(model, face_id, ModelDef::EDGES.size()); }
    };
    
    // Get all edges for a face
    FaceEdges face_edges(uint8_t face_id) const {
        return FaceEdges{this, face_id};
    }
    
    // === FaceProxy - provides the face-centric API user wants ===
    class FaceProxy {
    private:
        const Model* _model;
        Face* _face;
        uint8_t _face_id;
        
    public:
        FaceProxy(const Model* model, Face* face, uint8_t face_id) 
            : _model(model), _face(face), _face_id(face_id) {}
        
        // Forward Face methods
        uint8_t id() const { return _face->id(); }
        FaceType type() const { return _face->type(); }
        uint16_t led_offset() const { return _face->led_offset(); }
        uint16_t led_count() const { return _face->led_count(); }
        
        // Direct access to Face members
        Face::Leds& leds() { return _face->leds; }
        const Face::Leds& leds() const { return _face->leds; }
        Face::Vertices& vertices() { return _face->vertices; }
        const Face::Vertices& vertices() const { return _face->vertices; }
        
        // === Face-centric API (user requested design) ===
        
        /**
         * @brief Get LED group by name for this face
         */
        LedGroup group(const char* name) const {
            return _model->group(name, _face_id);
        }
        
        /**
         * @brief Get all available group names for this face
         */
        struct Groups {
            static constexpr size_t MAX_GROUPS = 10;
            const char* group_names[MAX_GROUPS];
            uint8_t count;
            
            struct Iterator {
                const char** names;
                uint8_t current;
                
                Iterator(const char** n, uint8_t curr) : names(n), current(curr) {}
                const char* operator*() const { return names[current]; }
                Iterator& operator++() { current++; return *this; }
                bool operator!=(const Iterator& other) const { return current != other.current; }
            };
            
            Iterator begin() const { return Iterator(const_cast<const char**>(group_names), 0); }
            Iterator end() const { return Iterator(const_cast<const char**>(group_names), count); }
            const char* operator[](size_t i) const { 
                return (i < count) ? group_names[i] : nullptr; 
            }
            size_t size() const { return count; }
        };
        
        Groups groups() const {
            Groups result;
            result.count = 0;
            
            // Search ModelDef for groups matching this face's type
            const auto& face_data = ModelDef::FACES[_face_id];
            for (size_t i = 0; i < ModelDef::LED_GROUPS.size() && result.count < Groups::MAX_GROUPS; i++) {
                const auto& group_data = ModelDef::LED_GROUPS[i];
                if (group_data.face_type_id == face_data.type_id) {
                    result.group_names[result.count] = group_data.name;
                    result.count++;
                }
            }
            return result;
        }
        
        /**
         * @brief Get edges for this face
         */
        FaceEdges edges() const {
            return _model->face_edges(_face_id);
        }
        
        /**
         * @brief Get the face connected at a specific edge
         */
        int8_t face_at_edge(uint8_t edge_index) const {
            return _model->face_at_edge(_face_id, edge_index);
        }
        
        /**
         * @brief Get number of edges for this face
         */
        uint8_t edge_count() const {
            return _model->face_edge_count(_face_id);
        }
    };
    
    // === USER REQUESTED API: face-centric access ===
    
    /**
     * @brief Get face proxy with rich API (USER REQUESTED DESIGN) 
     * @param geometric_position Geometric position (0-N) to access
     * @return FaceProxy for the face positioned at that geometric location
     */
    FaceProxy face(uint8_t geometric_position) {
        if (geometric_position >= ModelDef::FACE_COUNT) geometric_position = ModelDef::FACE_COUNT - 1;
        
        // Find the logical face that is positioned at the requested geometric position
        for (size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            if (ModelDef::FACES[i].geometric_id == geometric_position) {
                return FaceProxy(this, &_faces[i], static_cast<uint8_t>(i));
            }
        }
        
        // Fallback: if no face found at that geometric position, return the first face
        return FaceProxy(this, &_faces[0], 0);
    }
    
    const FaceProxy face(uint8_t geometric_position) const {
        if (geometric_position >= ModelDef::FACE_COUNT) geometric_position = ModelDef::FACE_COUNT - 1;
        
        // Find the logical face that is positioned at the requested geometric position
        for (size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            if (ModelDef::FACES[i].geometric_id == geometric_position) {
                return FaceProxy(this, const_cast<Face*>(&_faces[i]), static_cast<uint8_t>(i));
            }
        }
        
        // Fallback: if no face found at that geometric position, return the first face
        return FaceProxy(this, const_cast<Face*>(&_faces[0]), 0);
    }
    
    // Hardware metadata access
    struct Hardware {
        static constexpr const char* led_type() { return ModelDef::HARDWARE.led_type; }
        static constexpr const char* color_order() { return ModelDef::HARDWARE.color_order; }
        static constexpr float led_diameter_mm() { return ModelDef::HARDWARE.led_diameter_mm; }
        static constexpr float led_spacing_mm() { return ModelDef::HARDWARE.led_spacing_mm; }
        static constexpr uint16_t max_current_per_led_ma() { return ModelDef::HARDWARE.max_current_per_led_ma; }
        static constexpr uint16_t avg_current_per_led_ma() { return ModelDef::HARDWARE.avg_current_per_led_ma; }
    };
    
    // Access hardware metadata
    static constexpr Hardware hardware() { return Hardware{}; }
    
    // Size info
    static constexpr size_t led_count() { return ModelDef::LED_COUNT; }
    static constexpr size_t face_count() { return ModelDef::FACE_COUNT; }
    static constexpr size_t edge_count() { return ModelDef::EDGES.size(); }
    static constexpr size_t group_count() { return ModelDef::LED_GROUPS.size(); }
    
    // === IModel Interface Implementation ===
    
private:
    // Wrapper to adapt LedGroup to ILedGroup interface
    class LedGroupWrapper : public ILedGroup {
    private:
        LedGroup _group;
        
    public:
        explicit LedGroupWrapper(const LedGroup& group) : _group(group) {}
        
        CRGB& operator[](size_t i) override {
            return _group[i];
        }
        
        size_t size() const override {
            return _group.size();
        }
    };

public:
    // Implement IModel interface
    const Point& point(size_t index) const override {
        return points[index];
    }
    
    size_t pointCount() const noexcept override {
        return ModelDef::LED_COUNT;
    }
    
    const Face& face(size_t index) const override {
        return faces[index];
    }
    
    size_t faceCount() const noexcept override {
        return ModelDef::FACE_COUNT;
    }
    
    float getSphereRadius() const override {
        // Calculate bounding sphere radius from points
        float max_distance_sq = 0.0f;
        for (const auto& point : _points) {
            float distance_sq = point.x() * point.x() + point.y() * point.y() + point.z() * point.z();
            if (distance_sq > max_distance_sq) {
                max_distance_sq = distance_sq;
            }
        }
        return std::sqrt(max_distance_sq);
    }
    
    uint8_t face_edge_count(uint8_t geometric_position) const override {
        // Map geometric position to logical face ID
        uint8_t logical_face_id = geometric_position;
        for (size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            if (ModelDef::FACES[i].geometric_id == geometric_position) {
                logical_face_id = ModelDef::FACES[i].id;
                break;
            }
        }
        
        // Count edges for the logical face
        uint8_t count = 0;
        for (const auto& edge : ModelDef::EDGES) {
            if (edge.face_id == logical_face_id) {
                count++;
            }
        }
        return count;
    }
    
    int8_t face_at_edge(uint8_t geometric_position, uint8_t edge_index) const override {
        // Map geometric position to logical face ID
        uint8_t logical_face_id = geometric_position;
        for (size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            if (ModelDef::FACES[i].geometric_id == geometric_position) {
                logical_face_id = ModelDef::FACES[i].id;
                break;
            }
        }
        
        // Find the edge_index-th edge for the logical face
        uint8_t current_edge = 0;
        for (const auto& edge : ModelDef::EDGES) {
            if (edge.face_id == logical_face_id) {
                if (current_edge == edge_index) {
                    uint8_t connected_logical_id = (edge.connected_face_id == 255) ? 255 : edge.connected_face_id;
                    if (connected_logical_id == 255) {
                        return -1;
                    }
                    
                    // Map connected logical face back to geometric position
                    for (size_t j = 0; j < ModelDef::FACE_COUNT; j++) {
                        if (ModelDef::FACES[j].id == connected_logical_id) {
                            return static_cast<int8_t>(ModelDef::FACES[j].geometric_id);
                        }
                    }
                    return -1;
                }
                current_edge++;
            }
        }
        return -1; // Edge not found
    }
    
    std::unique_ptr<ILedGroup> face_group(uint8_t geometric_position, const char* group_name) const override {
        // Map geometric position to array index for the internal group() method
        uint8_t array_index = geometric_position;
        for (size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            if (ModelDef::FACES[i].geometric_id == geometric_position) {
                array_index = static_cast<uint8_t>(i);
                break;
            }
        }
        
        LedGroup group = this->group(group_name, array_index);
        return std::make_unique<LedGroupWrapper>(group);
    }
    
    std::vector<const char*> face_group_names(uint8_t geometric_position) const override {
        // Map geometric position to array index
        uint8_t array_index = geometric_position;
        for (size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            if (ModelDef::FACES[i].geometric_id == geometric_position) {
                array_index = static_cast<uint8_t>(i);
                break;
            }
        }
        
        if (array_index >= ModelDef::FACE_COUNT) {
            return std::vector<const char*>();
        }
        
        std::vector<const char*> result;
        const auto& face_data = ModelDef::FACES[array_index];
        
        // Search ModelDef for groups matching this face's type
        for (size_t i = 0; i < ModelDef::LED_GROUPS.size(); i++) {
            const auto& group_data = ModelDef::LED_GROUPS[i];
            if (group_data.face_type_id == face_data.type_id) {
                result.push_back(group_data.name);
            }
        }
        return result;
    }
};

} // namespace PixelTheater 