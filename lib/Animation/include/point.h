#pragma once
#include <cstdint>
#include <vector>

namespace Animation {

// Represents a point in normalized 3D space (-1.0 to 1.0)
struct Point {
    float x = 0;  // Normalized coordinates
    float y = 0;  // All values between -1.0 and 1.0
    float z = 0;  // Representing position on unit sphere
    
    Point() = default;
    Point(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    // Calculate Euclidean distance to another point
    float distanceTo(const Point& other) const;

    // Calculate dot product with another point
    float dotProduct(const Point& other) const;

    // Normalize this point to unit length
    void normalize();
};

} // namespace Animation 