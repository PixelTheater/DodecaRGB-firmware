#pragma once

#include <cstddef> // For size_t
#include <memory>  // For std::unique_ptr
#include <vector>  // For std::vector
#include "PixelTheater/model/point.h"
#include "PixelTheater/model/face.h"
#include "PixelTheater/core/crgb.h"

namespace PixelTheater {

// Forward Declarations
class Point;
class Face;

// LED Group interface
struct ILedGroup {
    virtual ~ILedGroup() = default;
    virtual CRGB& operator[](size_t i) = 0;
    virtual size_t size() const = 0;
    virtual bool empty() const { return size() == 0; }
};

// Edge data interface
struct IEdge {
    virtual ~IEdge() = default;
    virtual uint8_t face_id() const = 0;
    virtual uint8_t edge_index() const = 0;
    virtual int8_t connected_face_id() const = 0;
    virtual bool has_connection() const = 0;
};

// Face edges collection interface
struct IFaceEdges {
    virtual ~IFaceEdges() = default;
    // For now, just provide access by index - iteration can be added later
    virtual IEdge* edge_at(size_t index) const = 0;
    virtual size_t size() const = 0;
};

/**
 * @brief Interface for accessing 3D model geometry.
 * 
 * Provides indexed access to points and faces. Implementations should
 * handle invalid indices gracefully (e.g., clamping, dummy returns).
 */
class IModel {
public:
    virtual ~IModel() = default;
    virtual const Point& point(size_t index) const = 0;
    virtual size_t pointCount() const noexcept = 0;
    virtual const Face& face(size_t index) const = 0;
    virtual size_t faceCount() const noexcept = 0;
    virtual float getSphereRadius() const = 0;

    // === Face-Centric Methods ===
    
    virtual uint8_t face_edge_count(uint8_t face_id) const = 0;
    virtual int8_t face_at_edge(uint8_t face_id, uint8_t edge_index) const = 0;

    // === LED Group Access ===
    
    virtual std::unique_ptr<ILedGroup> face_group(uint8_t face_id, const char* group_name) const = 0;
    virtual std::vector<const char*> face_group_names(uint8_t face_id) const = 0;

    // === Model Validation ===
    struct ModelValidation {
        // Overall validation status
        bool is_valid;                    // True if all validations pass
        uint16_t total_checks;            // Total number of validation checks performed
        uint16_t failed_checks;           // Number of failed checks
        
        // Geometric validation results
        struct GeometricValidation {
            bool all_faces_planar;        // All faces have coplanar vertices
            bool all_leds_within_faces;   // All LEDs are within their face boundaries
            bool edge_connectivity_complete; // All edges have valid neighbors or are boundaries
            bool vertex_coordinates_sane; // No NaN/infinite/unreasonable coordinates
            bool led_coordinates_sane;    // LED coordinates are reasonable
            uint8_t non_planar_faces;     // Count of non-planar faces
            uint8_t misplaced_leds;       // Count of LEDs outside face boundaries
            uint8_t orphaned_edges;       // Count of edges without valid neighbors
            uint8_t invalid_coordinates;  // Count of invalid coordinate values
        } geometric;
        
        // Data integrity validation results
        struct DataIntegrityValidation {
            bool face_ids_unique;         // All face IDs are unique
            bool led_indices_sequential;  // LED indices are sequential and complete
            bool edge_data_complete;      // All edge data is present and valid
            bool vertex_data_complete;    // All vertex data is present and valid
            bool indices_in_bounds;       // All indices are within valid ranges
            uint8_t duplicate_face_ids;   // Count of duplicate face IDs
            uint8_t missing_edge_data;    // Count of missing edge entries
            uint8_t missing_vertex_data;  // Count of missing vertex entries
            uint8_t out_of_bounds_indices; // Count of out-of-bounds indices
        } data_integrity;
        
        // Detailed error information
        struct ErrorDetails {
            static constexpr size_t MAX_ERRORS = 10;
            char error_messages[MAX_ERRORS][128]; // Error message strings
            uint8_t error_count;                  // Number of error messages
            
            void add_error(const char* message) {
                if (error_count < MAX_ERRORS) {
                    // Copy message safely
                    size_t i = 0;
                    while (i < 127 && message[i] != '\0') {
                        error_messages[error_count][i] = message[i];
                        i++;
                    }
                    error_messages[error_count][i] = '\0';
                    error_count++;
                }
            }
        } errors;
        
        ModelValidation() : is_valid(false), total_checks(0), failed_checks(0) {
            // Initialize all validation fields
            geometric = {};
            data_integrity = {};
            errors = {};
        }
    };
    
    virtual ModelValidation validate_model(bool check_geometric_validity = true, 
                                         bool check_data_integrity = true) const = 0;

};

// Forward declaration for face-specific group access
struct IFaceGroup {
    virtual ~IFaceGroup() = default;
    virtual CRGB& operator[](size_t i) = 0;
    virtual size_t size() const = 0;
    virtual bool empty() const { return size() == 0; }
    virtual const char* name() const = 0;
};

} // namespace PixelTheater 