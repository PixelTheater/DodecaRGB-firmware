#include "point.h"
#include <cmath>

namespace Animation {

float Point::distanceTo(const Point& other) const {
    float dx = x - other.x;
    float dy = y - other.y;
    float dz = z - other.z;
    return sqrt(dx*dx + dy*dy + dz*dz);
}

float Point::dotProduct(const Point& other) const {
    return x*other.x + y*other.y + z*other.z;
}

void Point::normalize() {
    float len = sqrt(x*x + y*y + z*z);
    if (len > 0) {
        x /= len;
        y /= len;
        z /= len;
    }
}

} // namespace Animation 