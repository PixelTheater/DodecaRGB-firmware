#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)

#include "PixelTheater/platform/webgl/mesh.h"
#include <cmath>
#include <cstdio> // For printf instead of <iostream>
#include <map>
#include <set>
#include <array>
#include <utility>

namespace PixelTheater {
namespace WebGL {

void MeshGenerator::clear() {
    _vertices.clear();
    _indices.clear();
    _edge_vertices.clear();
    _edge_indices.clear();
}

void MeshGenerator::addVertex(float x, float y, float z, float nx, float ny, float nz) {
    // Add position
    _vertices.push_back(x);
    _vertices.push_back(y);
    _vertices.push_back(z);
    
    // Add normal
    _vertices.push_back(nx);
    _vertices.push_back(ny);
    _vertices.push_back(nz);
}

void MeshGenerator::addTriangle(uint16_t a, uint16_t b, uint16_t c) {
    _indices.push_back(a);
    _indices.push_back(b);
    _indices.push_back(c);
}

void MeshGenerator::addEdge(const WebVertex& v1, const WebVertex& v2) {
    // Add vertices for the edge line
    size_t start_idx = _edge_vertices.size() / 3;
    _edge_vertices.push_back(v1.x);
    _edge_vertices.push_back(v1.y);
    _edge_vertices.push_back(v1.z);
    _edge_vertices.push_back(v2.x);
    _edge_vertices.push_back(v2.y);
    _edge_vertices.push_back(v2.z);
    
    // Add indices for the line
    _edge_indices.push_back(start_idx);
    _edge_indices.push_back(start_idx + 1);
}

void MeshGenerator::calculateFaceNormal(const std::vector<WebVertex>& vertices, float& nx, float& ny, float& nz) {
    if (vertices.size() < 3) {
        nx = 0; ny = 0; nz = 1; // Default normal if not enough vertices
        return;
    }

    // Use first three vertices to calculate normal
    const WebVertex& v1 = vertices[0];
    const WebVertex& v2 = vertices[1];
    const WebVertex& v3 = vertices[2];

    // Calculate two vectors in the plane
    float ux = v2.x - v1.x;
    float uy = v2.y - v1.y;
    float uz = v2.z - v1.z;

    float vx = v3.x - v1.x;
    float vy = v3.y - v1.y;
    float vz = v3.z - v1.z;

    // Calculate cross product
    nx = uy * vz - uz * vy;
    ny = uz * vx - ux * vz;
    nz = ux * vy - uy * vx;

    // Normalize
    float length = std::sqrt(nx * nx + ny * ny + nz * nz);
    if (length > 0.0001f) {
        nx /= length;
        ny /= length;
        nz /= length;
    } else {
        nx = 0; ny = 0; nz = 1; // Default normal if vectors are parallel
    }
}

void MeshGenerator::generateDodecahedronMesh(const std::vector<WebFace>& faces) {
    clear();
    
    // Scale factor to match LED scaling - adjusted to match LED positions
    constexpr float POSITION_SCALE = 0.0295f;  // LED positions are pre-scaled, face vertices are in raw mm
    constexpr float Z_CORRECTION = 1.0f;
    
    // Generate faces and edges
    for (size_t face = 0; face < faces.size(); face++) {
        std::vector<WebVertex> faceVertices;
        float nx, ny, nz;
        
        // Get actual pentagon corner vertices for this face
        for (size_t i = 0; i < faces[face].vertices.size(); i++) {
            // Get vertex position from the provided face data
            float x = faces[face].vertices[i].x;
            float y = faces[face].vertices[i].y;
            float z = faces[face].vertices[i].z;
            
            // Scale positions to match LED scaling
            x *= POSITION_SCALE;
            y *= POSITION_SCALE;
            z *= POSITION_SCALE * Z_CORRECTION;
            
            faceVertices.push_back({x, y, z});
        }
        
        // Calculate face normal
        calculateFaceNormal(faceVertices, nx, ny, nz);
        
        // Calculate center point of pentagon
        WebVertex center = {0, 0, 0};
        for (const auto& v : faceVertices) {
            center.x += v.x;
            center.y += v.y;
            center.z += v.z;
        }
        center.x /= faceVertices.size();
        center.y /= faceVertices.size();
        center.z /= faceVertices.size();
        
        // Add center vertex
        size_t centerIndex = _vertices.size() / 6;
        addVertex(center.x, center.y, center.z, nx, ny, nz);
        
        // Add pentagon vertices
        size_t baseVertexIndex = _vertices.size() / 6;
        for (const auto& vertex : faceVertices) {
            addVertex(vertex.x, vertex.y, vertex.z, nx, ny, nz);
        }
        
        // Create triangles from center to each edge
        for (size_t i = 0; i < faceVertices.size(); i++) {
            size_t next = (i + 1) % faceVertices.size();
            // Create triangle with correct winding order
            addTriangle(
                centerIndex,
                baseVertexIndex + i,
                baseVertexIndex + next
            );
        }
        
        // Add edges between consecutive vertices
        for (size_t i = 0; i < faceVertices.size(); i++) {
            size_t next = (i + 1) % faceVertices.size();
            addEdge(faceVertices[i], faceVertices[next]);
        }
    }
    
    printf("Generated mesh with %zu vertices and %zu indices\n", 
           _vertices.size() / 6, _indices.size() / 3);
}

} // namespace WebGL
} // namespace PixelTheater

#endif // defined(PLATFORM_WEB) || defined(EMSCRIPTEN) 