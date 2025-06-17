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