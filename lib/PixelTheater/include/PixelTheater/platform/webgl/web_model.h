#pragma once

#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)

#include <string>
#include <vector>
#include <array>

namespace PixelTheater {

// Simple vertex type for web visualization
struct WebVertex {
    float x, y, z;
};

// Structure to hold face vertices for mesh generation
struct WebFace {
    std::array<WebVertex, 5> vertices;  // Pentagon has 5 vertices
};

// Basic information about the model
struct WebModelMetadata {
    std::string name;          
    std::string version;       
    uint16_t num_leds;        
};

// LED array information
struct WebLEDArray {
    std::vector<WebVertex> positions;  // Fixed 3D positions of each LED
};

// Physical model geometry 
struct WebGeometry {
    std::vector<WebFace> faces;       // Pentagon faces that make up the model
};

// Complete model description for web visualization
struct WebModel {
    WebModelMetadata metadata;
    WebLEDArray leds;
    WebGeometry geometry;
};

} // namespace PixelTheater

#endif // defined(PLATFORM_WEB) || defined(EMSCRIPTEN) 