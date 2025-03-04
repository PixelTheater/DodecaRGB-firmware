#pragma once

#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)

#include <vector>
#include <functional>
#include "PixelTheater/platform/webgl/web_model.h"
#include <cstdint>

namespace PixelTheater {
namespace WebGL {

// Type for coordinate provider function
using CoordinateProviderFunc = std::function<void(uint16_t, float&, float&, float&)>;

class MeshGenerator {
public:
    MeshGenerator() = default;
    ~MeshGenerator() = default;

    // Generate a dodecahedron mesh from the given faces
    void generateDodecahedronMesh(const std::vector<WebFace>& faces);

    // Clear all mesh data
    void clear();

    // Accessors for mesh data
    const std::vector<float>& getVertices() const { return _vertices; }
    const std::vector<uint16_t>& getIndices() const { return _indices; }
    const std::vector<float>& getEdgeVertices() const { return _edge_vertices; }
    const std::vector<uint16_t>& getEdgeIndices() const { return _edge_indices; }

private:
    // Calculate face normal from vertices
    void calculateFaceNormal(const std::vector<WebVertex>& vertices, float& nx, float& ny, float& nz);
    void calculateFaceNormal(const WebVertex& v1, const WebVertex& v2, const WebVertex& v3, float& nx, float& ny, float& nz);

    // Add a vertex with normal to the mesh
    void addVertex(float x, float y, float z, float nx, float ny, float nz);

    // Add a triangle to the mesh
    void addTriangle(uint16_t a, uint16_t b, uint16_t c);

    // Add an edge between two vertices
    void addEdge(const WebVertex& v1, const WebVertex& v2);

    // Mesh data
    std::vector<float> _vertices;         // Vertex positions and normals (x,y,z,nx,ny,nz)
    std::vector<uint16_t> _indices;       // Triangle indices
    std::vector<float> _edge_vertices;    // Edge vertex positions and normals
    std::vector<uint16_t> _edge_indices;  // Edge indices
};

} // namespace WebGL
} // namespace PixelTheater

#endif // defined(PLATFORM_WEB) || defined(EMSCRIPTEN) 