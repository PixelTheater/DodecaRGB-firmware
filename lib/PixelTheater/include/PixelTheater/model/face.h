#pragma once
#include <vector>
#include "PixelTheater/limits.h"  // For Config::Limits
#include "PixelTheater/core/math.h"
#include "PixelTheater/core/array_view.h"
#include "led.h"
#include "point.h"
#include "region.h"
#include "face_type.h"  // Use the single definition

namespace PixelTheater {

template<typename ModelDef> class Model;  // Forward declaration

class Face {
public:
    // Constructor takes face type and ID
    Face(FaceType type = FaceType::None, uint8_t id = 0) 
        : _id(id)
        , _type(type)
    {}

    // Basic properties
    FaceType type() const { return _type; }
    uint8_t id() const { return _id; }

    // Region access - these will be initialized by Model
    array_view<const Region> regions() const { 
        return array_view<const Region>(_regions.data(), _region_count); 
    }
    
    const Region& center() const { return _center; }
    
    array_view<const Region> rings() const {
        return array_view<const Region>(_rings.data(), _ring_count);
    }
    
    array_view<const Region> edges() const {
        return array_view<const Region>(_edges.data(), _edge_count);
    }

private:
    uint8_t _id;              
    FaceType _type;           
    uint8_t _rotation;        
    Point _position;          
    Eigen::Vector3d _normal;      

    // Region storage
    Region _center;                    // Single center region
    std::array<Region, 5> _rings;      // Max 5 rings (from requirements)
    std::array<Region, 8> _edges;      // Max 8 edges (octagon)
    size_t _ring_count{0};
    size_t _edge_count{0};
    
    // All regions (for iteration)
    std::array<Region, 14> _regions;   // 1 center + 5 rings + 8 edges
    size_t _region_count{0};

    template<typename T> friend class Model;  // Allow Model to initialize regions
};

} // namespace PixelTheater 