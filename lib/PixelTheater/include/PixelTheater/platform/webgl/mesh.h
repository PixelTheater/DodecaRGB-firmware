#pragma once

#include <cstdint>
#include <vector>
#include <functional>
#include <array>

namespace PixelTheater {

// Type for coordinate provider function
using CoordinateProviderFunc = std::function<void(uint16_t, float&, float&, float&)>;

// Classes to manage mesh generation
class MeshGenerator {
public:
    MeshGenerator(CoordinateProviderFunc coord_provider, uint16_t num_leds);
    
    // Generate a dodecahedron mesh from LED positions
    void generateDodecahedronMesh();
    
    // Get mesh data
    const std::vector<float>& getVertices() const { return _vertices; }
    const std::vector<uint16_t>& getIndices() const { return _indices; }
    
    // Reset the mesh 
    void clear();
    
private:
    CoordinateProviderFunc _coord_provider;
    uint16_t _num_leds;
    std::vector<float> _vertices;     // x, y, z, nx, ny, nz for each vertex
    std::vector<uint16_t> _indices;   // Triangle indices
    
    // Helper methods
    void addVertex(float x, float y, float z, float nx, float ny, float nz);
    void addTriangle(uint16_t a, uint16_t b, uint16_t c);
};

} // namespace PixelTheater 