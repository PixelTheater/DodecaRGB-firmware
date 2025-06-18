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

    /**
     * @brief LED array accessor providing bounds-checked access to all LEDs
     * 
     * Provides array-like interface to the LED color data with automatic bounds checking.
     * Supports iteration and random access to individual LEDs.
     * 
     * Example usage:
     * ```cpp
     * model.leds[0] = CRGB::Red;           // Set first LED
     * for (auto& led : model.leds) {       // Iterate all LEDs  
     *     led = CRGB::Black;               // Clear all LEDs
     * }
     * ```
     */
    struct Leds {
        CRGB* _data;  // Pointer to LED array
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
        
        // Iterator support for range-based loops
        auto begin() { return _data; }
        auto end() { return _data + _size; }
        auto begin() const { return _data; }
        auto end() const { return _data + _size; }
    } leds{_leds, ModelDef::LED_COUNT};

    /**
     * @brief Point array accessor providing access to 3D geometry data
     * 
     * Provides access to 3D coordinate data for each LED position. Each point
     * corresponds to an LED at the same index and contains x,y,z coordinates,
     * face assignment, and neighbor information.
     * 
     * Example usage:
     * ```cpp
     * const Point& p = model.points[42];   // Get point for LED 42
     * float height = p.z();               // Get Z coordinate
     * auto neighbors = p.getNeighbors();  // Get neighboring LEDs
     * ```
     */
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

        // Iterator support for range-based loops
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
        
        /**
         * @brief Calculate the center point of an edge
         * @param edge_index Edge index (0 to edge_count()-1)
         * @return Vertex representing the midpoint of the edge
         */
        Vertex edge_center(uint8_t edge_index) const {
            if (edge_index >= vertices().size()) {
                return Vertex{0.0f, 0.0f, 0.0f};
            }
            
            // Get current and next vertex (wrapping around)
            const auto& v0 = vertices()[edge_index];
            const auto& v1 = vertices()[(edge_index + 1) % vertices().size()];
            
            // Return midpoint
            return Vertex{
                (v0.x + v1.x) / 2.0f,
                (v0.y + v1.y) / 2.0f,
                (v0.z + v1.z) / 2.0f
            };
        }
        
        /**
         * @brief Find LEDs on this face that are near a specific 3D point
         * @param point The 3D point to search around
         * @param max_distance Maximum distance to consider "nearby"
         * @return Vector of LED indices and their distances, sorted by distance
         */
        struct NearbyLed {
            uint16_t led_index;  // Global LED index
            float distance;      // Distance to the point
        };
        
        std::vector<NearbyLed> nearby_leds(const Vertex& point, float max_distance = std::numeric_limits<float>::max()) const {
            std::vector<NearbyLed> result;
            
            // Get face LED range
            uint16_t face_led_offset = led_offset();
            uint16_t face_led_count = led_count();
            
            // Check each LED on this face
            for (uint16_t face_led_idx = 0; face_led_idx < face_led_count; face_led_idx++) {
                uint16_t global_led_idx = face_led_offset + face_led_idx;
                const auto& led_point = _model->point(global_led_idx);
                
                // Calculate distance from LED to target point
                float distance = led_point.distanceTo(point);
                
                // Add to results if within max distance
                if (distance <= max_distance) {
                    result.push_back({global_led_idx, distance});
                }
            }
            
            // Sort by distance (closest first)
            std::sort(result.begin(), result.end(),
                     [](const NearbyLed& a, const NearbyLed& b) {
                         return a.distance < b.distance;
                     });
            
            return result;
        }
        
        /**
         * @brief Calculate midpoint between two specific vertices
         * @param vertex_a_index Index of first vertex
         * @param vertex_b_index Index of second vertex  
         * @return Vertex representing the midpoint between the two vertices
         */
        Vertex vertex_midpoint(uint8_t vertex_a_index, uint8_t vertex_b_index) const {
            if (vertex_a_index >= vertices().size() || vertex_b_index >= vertices().size()) {
                return Vertex{0.0f, 0.0f, 0.0f};
            }
            
            const auto& va = vertices()[vertex_a_index];
            const auto& vb = vertices()[vertex_b_index];
            
            return Vertex{
                (va.x + vb.x) / 2.0f,
                (va.y + vb.y) / 2.0f,
                (va.z + vb.z) / 2.0f
            };
        }
        
        /**
         * @brief Basic geometry validation for this face
         * @return Structure containing validation results
         */
        struct GeometryValidation {
            bool has_vertices;          // Face has vertices defined
            bool has_leds;             // Face has LEDs defined  
            bool vertices_reasonable;   // Vertex coordinates are reasonable
            bool leds_reasonable;      // LED coordinates are reasonable
            float face_radius;         // Approximate radius of face
            uint8_t vertex_count;      // Number of vertices
            uint16_t led_count;        // Number of LEDs
        };
        
        GeometryValidation validate_geometry() const {
            GeometryValidation result = {};
            
            // Check vertices
            result.vertex_count = static_cast<uint8_t>(vertices().size());
            result.has_vertices = (result.vertex_count >= 3);
            
            // Check LEDs  
            result.led_count = led_count();
            result.has_leds = (result.led_count > 0);
            
            // Validate vertex coordinates are reasonable
            result.vertices_reasonable = true;
            float max_coord = 0.0f;
            if (result.has_vertices) {
                for (size_t i = 0; i < result.vertex_count; i++) {
                    const auto& v = vertices()[i];
                    float coord_magnitude = std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
                    max_coord = std::max(max_coord, coord_magnitude);
                    
                    // Check for unreasonable coordinates (NaN, infinity, too large)
                    if (!std::isfinite(v.x) || !std::isfinite(v.y) || !std::isfinite(v.z) || 
                        coord_magnitude > 10000.0f) {
                        result.vertices_reasonable = false;
                        break;
                    }
                }
            }
            
            // Validate LED coordinates are reasonable
            result.leds_reasonable = true;
            if (result.has_leds) {
                uint16_t face_led_offset = led_offset();
                for (uint16_t i = 0; i < result.led_count; i++) {
                    const auto& led_point = _model->point(face_led_offset + i);
                    float coord_magnitude = std::sqrt(led_point.x()*led_point.x() + 
                                                     led_point.y()*led_point.y() + 
                                                     led_point.z()*led_point.z());
                    
                    if (!std::isfinite(led_point.x()) || !std::isfinite(led_point.y()) || 
                        !std::isfinite(led_point.z()) || coord_magnitude > 10000.0f) {
                        result.leds_reasonable = false;
                        break;
                    }
                }
            }
            
            // Estimate face radius (distance from center to furthest vertex)
            result.face_radius = 0.0f;
            if (result.has_vertices && result.vertices_reasonable) {
                // Calculate center of vertices
                float center_x = 0.0f, center_y = 0.0f, center_z = 0.0f;
                for (size_t i = 0; i < result.vertex_count; i++) {
                    const auto& v = vertices()[i];
                    center_x += v.x;
                    center_y += v.y; 
                    center_z += v.z;
                }
                center_x /= result.vertex_count;
                center_y /= result.vertex_count;
                center_z /= result.vertex_count;
                
                // Find max distance from center to vertex
                for (size_t i = 0; i < result.vertex_count; i++) {
                    const auto& v = vertices()[i];
                    float dx = v.x - center_x;
                    float dy = v.y - center_y;
                    float dz = v.z - center_z;
                    float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
                    result.face_radius = std::max(result.face_radius, distance);
                }
            }
            
            return result;
        }
    };
    
    // === Face-Centric Access API ===
    
    /**
     * @brief Get face proxy with rich face-centric API
     * 
     * Returns a FaceProxy that provides convenient access to face-specific
     * functionality including LEDs, groups, edges, and geometric operations.
     * 
     * @param geometric_position Geometric position (0 to faceCount()-1)
     * @return FaceProxy for the face at that geometric position
     * 
     * Example usage:
     * ```cpp
     * auto face = model.face(0);              // Get bottom face
     * face.leds()[0] = CRGB::Red;            // Light first LED
     * auto center = face.group("center");    // Get center LEDs
     * for (auto& led : center) {             // Light center group
     *     led = CRGB::White;
     * }
     * ```
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
    
    /**
     * @brief Get face proxy with rich face-centric API (const version)
     * @param geometric_position Geometric position (0 to faceCount()-1)
     * @return Const FaceProxy for the face at that geometric position
     */
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
    
    /**
     * @brief Hardware metadata accessor
     * 
     * Provides access to hardware specifications defined in the model configuration.
     * All values are compile-time constants from the model definition.
     */
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

    /**
     * @brief Comprehensive model validation implementation
     */
    ModelValidation validate_model(bool check_geometric_validity = true, 
                                 bool check_data_integrity = true) const override {
        ModelValidation result;
        
        // Data integrity validation
        if (check_data_integrity) {
            validate_data_integrity(result);
        }
        
        // Geometric validation (more expensive)
        if (check_geometric_validity) {
            validate_geometric_integrity(result);
        }
        
        // Calculate overall validation status
        result.is_valid = (result.failed_checks == 0);
        
        return result;
    }

private:
    /**
     * @brief Validate data integrity aspects of the model
     */
    void validate_data_integrity(ModelValidation& result) const {
        auto& data = result.data_integrity;
        
        // Check face ID uniqueness
        result.total_checks++;
        bool face_ids_unique = true;
        for (size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            uint8_t face_id = _faces[i].id();
            for (size_t j = i + 1; j < ModelDef::FACE_COUNT; j++) {
                if (_faces[j].id() == face_id) {
                    face_ids_unique = false;
                    data.duplicate_face_ids++;
                    result.errors.add_error("Duplicate face ID found");
                }
            }
        }
        data.face_ids_unique = face_ids_unique;
        if (!face_ids_unique) result.failed_checks++;
        
        // Check LED indices are sequential and complete
        result.total_checks++;
        bool led_indices_sequential = true;
        size_t expected_offset = 0;
        for (size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            const auto& face = _faces[i];
            if (face.led_offset() != expected_offset) {
                led_indices_sequential = false;
                result.errors.add_error("LED indices not sequential");
                break;
            }
            expected_offset += face.led_count();
        }
        data.led_indices_sequential = led_indices_sequential;
        if (!led_indices_sequential) result.failed_checks++;
        
        // Check edge data completeness
        result.total_checks++;
        data.edge_data_complete = true;
        data.missing_edge_data = 0;
        for (size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            uint8_t logical_face_id = _faces[i].id();
            uint8_t expected_edges = static_cast<uint8_t>(_faces[i].type());
            uint8_t found_edges = 0;
            
            for (const auto& edge : ModelDef::EDGES) {
                if (edge.face_id == logical_face_id) {
                    found_edges++;
                }
            }
            
            if (found_edges < expected_edges) {
                data.edge_data_complete = false;
                data.missing_edge_data += (expected_edges - found_edges);
                result.errors.add_error("Missing edge data for face");
            }
        }
        if (!data.edge_data_complete) result.failed_checks++;
        
        // Check vertex data completeness
        result.total_checks++;
        data.vertex_data_complete = true;
        data.missing_vertex_data = 0;
        for (size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            const auto& face = _faces[i];
            uint8_t expected_vertices = static_cast<uint8_t>(face.type());
            
            // Count non-zero vertices (assuming zero vertices indicate missing data)
            uint8_t valid_vertices = 0;
            for (size_t j = 0; j < expected_vertices; j++) {
                const auto& vertex = face.vertices[j];
                if (vertex.x != 0.0f || vertex.y != 0.0f || vertex.z != 0.0f) {
                    valid_vertices++;
                }
            }
            
            if (valid_vertices < expected_vertices) {
                data.vertex_data_complete = false;
                data.missing_vertex_data += (expected_vertices - valid_vertices);
                result.errors.add_error("Missing vertex data for face");
            }
        }
        if (!data.vertex_data_complete) result.failed_checks++;
        
        // Check indices are within bounds
        result.total_checks++;
        data.indices_in_bounds = true;
        data.out_of_bounds_indices = 0;
        for (size_t i = 0; i < ModelDef::LED_COUNT; i++) {
            const auto& point = _points[i];
            if (point.face_id() >= ModelDef::FACE_COUNT) {
                data.indices_in_bounds = false;
                data.out_of_bounds_indices++;
                result.errors.add_error("Point face_id out of bounds");
            }
        }
        if (!data.indices_in_bounds) result.failed_checks++;
    }
    
    /**
     * @brief Validate geometric integrity aspects of the model
     */
    void validate_geometric_integrity(ModelValidation& result) const {
        auto& geom = result.geometric;
        
        // Check face planarity (all vertices coplanar)
        result.total_checks++;
        geom.all_faces_planar = true;
        geom.non_planar_faces = 0;
        for (size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            if (!is_face_planar(i)) {
                geom.all_faces_planar = false;
                geom.non_planar_faces++;
                result.errors.add_error("Face vertices not coplanar");
            }
        }
        if (!geom.all_faces_planar) result.failed_checks++;
        
        // Check vertex coordinate sanity
        result.total_checks++;
        geom.vertex_coordinates_sane = true;
        geom.invalid_coordinates = 0;
        for (size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            const auto& face = _faces[i];
            uint8_t vertex_count = static_cast<uint8_t>(face.type());
            
            for (size_t j = 0; j < vertex_count; j++) {
                const auto& vertex = face.vertices[j];
                if (!are_coordinates_sane(vertex.x, vertex.y, vertex.z)) {
                    geom.vertex_coordinates_sane = false;
                    geom.invalid_coordinates++;
                    result.errors.add_error("Invalid vertex coordinates");
                }
            }
        }
        if (!geom.vertex_coordinates_sane) result.failed_checks++;
        
        // Check LED coordinate sanity
        result.total_checks++;
        geom.led_coordinates_sane = true;
        for (size_t i = 0; i < ModelDef::LED_COUNT; i++) {
            const auto& point = _points[i];
            if (!are_coordinates_sane(point.x(), point.y(), point.z())) {
                geom.led_coordinates_sane = false;
                geom.invalid_coordinates++;
                result.errors.add_error("Invalid LED coordinates");
            }
        }
        if (!geom.led_coordinates_sane) result.failed_checks++;
        
        // Check LED placement within face boundaries (simplified check)
        result.total_checks++;
        geom.all_leds_within_faces = true;
        geom.misplaced_leds = 0;
        for (size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
            if (!are_face_leds_reasonable(i)) {
                geom.all_leds_within_faces = false;
                geom.misplaced_leds++;
                result.errors.add_error("LEDs positioned unreasonably relative to face");
            }
        }
        if (!geom.all_leds_within_faces) result.failed_checks++;
        
        // Check edge connectivity completeness
        result.total_checks++;
        geom.edge_connectivity_complete = true;
        geom.orphaned_edges = 0;
        for (const auto& edge : ModelDef::EDGES) {
            // Check if connected face exists (when connection is claimed)
            if (edge.connected_face_id != 255) { // 255 typically indicates no connection
                bool connected_face_exists = false;
                for (size_t i = 0; i < ModelDef::FACE_COUNT; i++) {
                    if (_faces[i].id() == edge.connected_face_id) {
                        connected_face_exists = true;
                        break;
                    }
                }
                if (!connected_face_exists) {
                    geom.edge_connectivity_complete = false;
                    geom.orphaned_edges++;
                    result.errors.add_error("Edge references non-existent connected face");
                }
            }
        }
        if (!geom.edge_connectivity_complete) result.failed_checks++;
    }
    
    /**
     * @brief Helper: Check if face vertices are coplanar
     */
    bool is_face_planar(size_t face_index) const {
        if (face_index >= ModelDef::FACE_COUNT) return false;
        
        const auto& face = _faces[face_index];
        uint8_t vertex_count = static_cast<uint8_t>(face.type());
        
        if (vertex_count < 4) return true; // Triangles are always planar
        
        // Use first three vertices to define the plane
        const auto& v0 = face.vertices[0];
        const auto& v1 = face.vertices[1]; 
        const auto& v2 = face.vertices[2];
        
        // Calculate plane normal vector (v1-v0) × (v2-v0)
        float n_x = (v1.y - v0.y) * (v2.z - v0.z) - (v1.z - v0.z) * (v2.y - v0.y);
        float n_y = (v1.z - v0.z) * (v2.x - v0.x) - (v1.x - v0.x) * (v2.z - v0.z);
        float n_z = (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);
        
        // Normalize
        float n_mag = std::sqrt(n_x*n_x + n_y*n_y + n_z*n_z);
        if (n_mag < 1e-6f) return false; // Degenerate case
        
        n_x /= n_mag;
        n_y /= n_mag; 
        n_z /= n_mag;
        
        // Calculate plane distance: d = -n·v0
        float d = -(n_x * v0.x + n_y * v0.y + n_z * v0.z);
        
        // Check if all other vertices lie on this plane
        const float tolerance = 1.0f; // Allow 1 unit tolerance
        for (size_t i = 3; i < vertex_count; i++) {
            const auto& v = face.vertices[i];
            float distance = std::abs(n_x * v.x + n_y * v.y + n_z * v.z + d);
            if (distance > tolerance) {
                return false;
            }
        }
        
        return true;
    }
    
    /**
     * @brief Helper: Check if coordinates are reasonable (not NaN, infinite, etc.)
     */
    bool are_coordinates_sane(float x, float y, float z) const {
        const float max_reasonable = 10000.0f;
        
        return std::isfinite(x) && std::isfinite(y) && std::isfinite(z) &&
               std::abs(x) <= max_reasonable && 
               std::abs(y) <= max_reasonable && 
               std::abs(z) <= max_reasonable;
    }
    
    /**
     * @brief Helper: Check if LEDs are positioned reasonably relative to face
     */
    bool are_face_leds_reasonable(size_t face_index) const {
        if (face_index >= ModelDef::FACE_COUNT) return false;
        
        const auto& face = _faces[face_index];
        
        // For validation purposes, use the face's own vertices
        // Face remapping is handled at the access level, not validation level
        const auto* vertex_face = &face;
        
        // Calculate face center from the geometrically correct vertices
        uint8_t vertex_count = static_cast<uint8_t>(vertex_face->type());
        float center_x = 0.0f, center_y = 0.0f, center_z = 0.0f;
        
        for (size_t i = 0; i < vertex_count; i++) {
            const auto& v = vertex_face->vertices[i];
            center_x += v.x;
            center_y += v.y;
            center_z += v.z;
        }
        center_x /= vertex_count;
        center_y /= vertex_count;
        center_z /= vertex_count;
        
        // Calculate maximum distance from center to vertices (face "radius")
        float max_vertex_distance = 0.0f;
        for (size_t i = 0; i < vertex_count; i++) {
            const auto& v = vertex_face->vertices[i];
            float dx = v.x - center_x;
            float dy = v.y - center_y;
            float dz = v.z - center_z;
            float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
            max_vertex_distance = std::max(max_vertex_distance, distance);
        }
        
        // Check if all LEDs are within reasonable distance of face center
        // Allow LEDs to be up to 2x the face radius from center (generous tolerance for validation)
        float reasonable_distance = max_vertex_distance * 2.0f;
        
        for (uint16_t i = 0; i < face.led_count(); i++) {
            uint16_t global_led_index = face.led_offset() + i;
            const auto& led_point = _points[global_led_index];
            
            float dx = led_point.x() - center_x;
            float dy = led_point.y() - center_y;
            float dz = led_point.z() - center_z;
            float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
            
            if (distance > reasonable_distance) {
                return false;
            }
        }
        
        return true;
    }

public:
};

} // namespace PixelTheater 