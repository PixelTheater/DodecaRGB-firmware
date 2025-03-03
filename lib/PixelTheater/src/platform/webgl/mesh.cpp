#include "PixelTheater/platform/webgl/mesh.h"
#include <cmath>
#include <cstdio> // For printf instead of <iostream>
#include <map>
#include <set>
#include <array>
#include <utility>

namespace PixelTheater {

MeshGenerator::MeshGenerator(std::function<void(uint16_t, float&, float&, float&)> coordProvider, uint16_t numLEDs)
    : _coord_provider(coordProvider), _num_leds(numLEDs) {
    // Empty constructor
}

void MeshGenerator::generateDodecahedronMesh() {
    // Clear any existing mesh data
    clear();
    
    // The golden ratio - key to building a dodecahedron
    const float phi = (1.0f + std::sqrt(5.0f)) / 2.0f;
    
    // The scale factor for the dodecahedron
    const float scale = 250.0f;
    
    // Vertices of a regular dodecahedron
    std::vector<std::array<float, 3>> vertices = {
        // (±1, ±1, ±1)
        {-1.0f, -1.0f, -1.0f},
        {-1.0f, -1.0f,  1.0f},
        {-1.0f,  1.0f, -1.0f},
        {-1.0f,  1.0f,  1.0f},
        { 1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f,  1.0f},
        { 1.0f,  1.0f, -1.0f},
        { 1.0f,  1.0f,  1.0f},
        
        // (0, ±1/φ, ±φ)
        {0.0f, -1.0f/phi, -phi},
        {0.0f, -1.0f/phi,  phi},
        {0.0f,  1.0f/phi, -phi},
        {0.0f,  1.0f/phi,  phi},
        
        // (±1/φ, ±φ, 0)
        {-1.0f/phi, -phi, 0.0f},
        {-1.0f/phi,  phi, 0.0f},
        { 1.0f/phi, -phi, 0.0f},
        { 1.0f/phi,  phi, 0.0f},
        
        // (±φ, 0, ±1/φ)
        {-phi, 0.0f, -1.0f/phi},
        {-phi, 0.0f,  1.0f/phi},
        { phi, 0.0f, -1.0f/phi},
        { phi, 0.0f,  1.0f/phi}
    };
    
    // Scale vertices
    for (auto& v : vertices) {
        v[0] *= scale;
        v[1] *= scale;
        v[2] *= scale;
    }
    
    // Define the 12 faces of the dodecahedron, each with 5 vertex indices
    const std::array<std::array<int, 5>, 12> faces = {{
        {0, 8, 10, 2, 16},    // Face 0
        {0, 12, 14, 4, 8},    // Face 1
        {0, 16, 17, 1, 12},   // Face 2
        {1, 9, 5, 14, 12},    // Face 3
        {1, 17, 3, 11, 9},    // Face 4
        {2, 10, 6, 15, 13},   // Face 5
        {2, 13, 3, 17, 16},   // Face 6
        {3, 13, 15, 7, 11},   // Face 7
        {4, 14, 5, 19, 18},   // Face 8
        {4, 18, 6, 10, 8},    // Face 9
        {5, 9, 11, 7, 19},    // Face 10
        {6, 18, 19, 7, 15}    // Face 11
    }};
    
    // Add each face to the mesh
    for (const auto& face : faces) {
        // Calculate face center and normal for proper lighting
        std::array<float, 3> center = {0.0f, 0.0f, 0.0f};
        
        for (int idx : face) {
            const auto& v = vertices[idx];
            center[0] += v[0];
            center[1] += v[1];
            center[2] += v[2];
        }
        
        center[0] /= 5.0f;
        center[1] /= 5.0f;
        center[2] /= 5.0f;
        
        // Calculate normal as direction from origin to center (for dodecahedron)
        float normalLength = std::sqrt(center[0]*center[0] + center[1]*center[1] + center[2]*center[2]);
        std::array<float, 3> normal = {
            center[0] / normalLength,
            center[1] / normalLength,
            center[2] / normalLength
        };
        
        // Add vertices with calculated normal
        std::vector<uint16_t> faceIndices;
        for (int idx : face) {
            // Store result of addVertex
            uint16_t vertexIndex = 0;
            // Use the void function form
            addVertex(
                vertices[idx][0], vertices[idx][1], vertices[idx][2],
                normal[0], normal[1], normal[2]
            );
            // Get the last vertex index
            vertexIndex = static_cast<uint16_t>(_vertices.size() / 6 - 1);
            faceIndices.push_back(vertexIndex);
        }
        
        // Triangulate the pentagon (fan triangulation)
        for (int i = 1; i < 4; ++i) {
            // Use addTriangle as declared in the header
            addTriangle(faceIndices[0], faceIndices[i], faceIndices[i+1]);
        }
    }
}

void MeshGenerator::addVertex(float x, float y, float z, float nx, float ny, float nz) {
    _vertices.push_back(x);
    _vertices.push_back(y);
    _vertices.push_back(z);
    _vertices.push_back(nx);
    _vertices.push_back(ny);
    _vertices.push_back(nz);
}

void MeshGenerator::addTriangle(uint16_t a, uint16_t b, uint16_t c) {
    _indices.push_back(a);
    _indices.push_back(b);
    _indices.push_back(c);
}

void MeshGenerator::clear() {
    _vertices.clear();
    _indices.clear();
}

} // namespace PixelTheater 