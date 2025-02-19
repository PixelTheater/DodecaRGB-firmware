# PixelTheater Development Plan

Reference docs:
‼️ @model.md : Model structure and data format documentation
‼️ @scene.md : Scene class documentation

## Development Plan

### 1. Core Data Structures

✓ Model Definition Format

- ✓ Define constexpr data structures
  - ✓ FaceTypeData
  - ✓ FaceData
  - ✓ PointData
  - ✓ RegionData
  - ✓ NeighborData
- ✓ Update ModelDefinition template
- ✓ Add compile-time validation

### 2. Test Fixtures

1. Basic Models

- [✓] NAME, VERSION, DESCRIPTION
- [✓] model_type (for generator strategy)
- [✓] Test-driven model validation
  - [✓] Metadata validation
  - [✓] Basic face configuration
  - [✓] Point geometry validation
  - [ ] Core relationship implementation
    - [ ] Model to Face access
    - [ ] Face to Region access
    - [ ] Region to LED access
    - [ ] LED to Point access

2. Test Cases

- ✓ Basic enum value validation
- ✓ Type consistency checks
- [✓] Metadata validation
- [ ] Collection access patterns
  - [ ] Model.faces() access
  - [ ] Face.regions() access
  - [ ] Region.leds() access
  - [ ] LED/Point relationships

Next Steps:

A. Implement core Model class

- Add basic collection access methods
- Define relationships between components
- Create minimal test fixture data
- Test access patterns work as expected

B. Implement Face and Region classes

- Add collection access methods
- Test relationship with Model class
- Verify LED access patterns

C. Future Tasks (After Core Implementation)

- Validate face requirements
- Add region completeness checks
- Implement neighbor relationships
- Add geometric validation

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
