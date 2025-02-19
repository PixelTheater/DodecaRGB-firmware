#pragma once
#include <vector>
#include "../core/math.h"
#include "../led.h"
#include "../limits.h"  // For Config::Limits
#include "point.h"
#include "region.h"

namespace PixelTheater {

// Just the basic geometric types
enum class FaceType {
    None = 0,
    Strip = 1,
    Circle = 2,
    Triangle = 3,
    Square = 4,
    Pentagon = 5,
    Hexagon = 6
};

class Face {
private:
    uint8_t _id;              
    FaceType _type;           
    uint8_t _rotation;        
    Point _position;          
    Eigen::Vector3d _normal;      


public:
    // Add default constructor
    Face() = default;
    
  
};

} // namespace PixelTheater 