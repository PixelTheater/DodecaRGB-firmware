# PixelTheater Development Plan

Reference docs:
‼️ @model.md : Model structure and data format documentation
‼️ @scene.md : Scene class documentation

## Development Plan

### 1. Core Data Structures

1. Model Definition Format

- [ ] Define constexpr data structures:
  - FaceTypeData (num_leds, edge_length, region counts)
  - FaceData (id, type_id, rotation, position)
  - PointData (id, face_id, position)
  - RegionData (id, face_id, type, led_ids)
  - NeighborData (point_id, neighbor distances)
- [ ] Update ModelDefinition template
- [ ] Add compile-time validation

2. Test Fixtures

- [ ] Update BasicPentagonModel with complete data
- [ ] Add validation test cases
- [ ] Test neighbor relationships

### 2. Runtime Classes

1. Point & LED Implementation

- [ ] Update Point class for new format
- [ ] Implement LED array initialization
- [ ] Add neighbor access methods

2. Region Implementation

- [ ] Region class with LED collections
- [ ] Face region grouping (center, rings, edges)
- [ ] Region validation (completeness, uniqueness)

3. Face Implementation

- [ ] Face type properties and validation
- [ ] Face position and normal calculation
- [ ] Region access methods

4. Model Implementation

- [ ] LED/Point array management
- [ ] Face collection and lookup
- [ ] Region access and validation

### 3. Testing Strategy

1. Unit Tests

- [ ] Data structure validation
- [ ] Point/LED relationships
- [ ] Region completeness
- [ ] Face geometry
- [ ] Model construction

2. Integration Tests

- [ ] Complete model initialization
- [ ] LED access patterns
- [ ] Region operations
- [ ] Neighbor traversal

Next Steps:

- Implement core data structures
- Create test fixtures
- Build runtime classes

## Future Considerations

### Teensy41 Platform Investigation

Current Issues:

- Binary.h macro conflicts with Eigen
- ARM GCC iterator differences from native
- Memory model differences in STL containers

### Math Library Evaluation

Current: Using Eigen for Vector3d normals in Face class

Required Operations:

- Basic: Vector3, Matrix3x3, rotations (currently only using Vector3d)
- Spherical: coordinate conversion, surface mapping, wave propagation
- Motion: quaternions, IMU integration (9-axis sensor support)

Options:

- Keep Eigen but wrap in our interfaces
- Create minimal math lib for common ops
- Hybrid: basic ops in-house + optional Eigen

Next Steps:

1. Profile Eigen memory usage on Teensy
2. Document animation math requirements
3. Test basic ops performance vs Eigen
