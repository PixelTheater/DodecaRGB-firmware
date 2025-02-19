#pragma once
#include <vector>
#include "../core/math.h"
#include "../led.h"
#include "../limits.h"  // For Config::Limits
#include "point.h"
#include "region.h"
#include "face_type.h"  // Use the single definition

namespace PixelTheater {

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