#pragma once

#include <cstddef> // For size_t
#include "PixelTheater/model/point.h"
#include "PixelTheater/model/face.h"

namespace PixelTheater {

// Forward Declarations
class Point;
class Face;

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

};

} // namespace PixelTheater 