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

    /**
     * @brief Get a const reference to the Point at the specified index.
     * Corresponds to the LED at the same index.
     * @param index The index (typically 0 to pointCount()-1).
     * @return Const reference to the Point. 
     * Behavior for invalid index is implementation-defined (e.g., clamping).
     */
    virtual const Point& point(size_t index) const = 0;

    /**
     * @brief Get the total number of points (should match ledCount from ILedBuffer).
     */
    virtual size_t pointCount() const noexcept = 0;

    /**
     * @brief Get a const reference to the Face at the specified index.
     * @param index The index (0 to faceCount()-1).
     * @return Const reference to the Face.
     * Behavior for invalid index is implementation-defined (e.g., clamping).
     */
    virtual const Face& face(size_t index) const = 0;

    /**
     * @brief Get the total number of faces in the model.
     */
    virtual size_t faceCount() const noexcept = 0;

    /**
     * @brief Get the calculated radius of the sphere encompassing the model.
     * @return The sphere radius (typically in the same units as point coordinates).
     */
    virtual float getSphereRadius() const = 0;

    // === Face-Centric Methods ===
    
    /**
     * @brief Get the number of edges for a specific face.
     * @param face_id The face ID.
     * @return Number of edges for the face.
     */
    virtual uint8_t face_edge_count(uint8_t face_id) const = 0;

    /**
     * @brief Get the face ID connected to a specific edge of a face.
     * @param face_id The face ID.
     * @param edge_index The edge index within the face (0-based).
     * @return Connected face ID, or -1 if no connection or invalid edge.
     */
    virtual int8_t face_at_edge(uint8_t face_id, uint8_t edge_index) const = 0;

    // === LED Group Access ===
    
    /**
     * @brief Get LED group by name for a specific face
     * @param face_id The face ID
     * @param group_name The name of the group (e.g., "center", "edge0", "ring1")
     * @return LED group interface for iteration and access
     */
    virtual std::unique_ptr<ILedGroup> face_group(uint8_t face_id, const char* group_name) const = 0;
    
    /**
     * @brief Get all available group names for a specific face
     * @param face_id The face ID
     * @return Vector of group names available for this face
     */
    virtual std::vector<const char*> face_group_names(uint8_t face_id) const = 0;

    // === Model Validation ===
    
    /**
     * @brief Comprehensive model validation results
     */
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
    
    /**
     * @brief Perform comprehensive model validation
     * @param check_geometric_validity If true, perform expensive geometric checks
     * @param check_data_integrity If true, perform data integrity checks
     * @return Detailed validation results
     */
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