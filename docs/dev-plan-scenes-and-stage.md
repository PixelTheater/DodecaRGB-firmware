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
  - [✓] Core relationship implementation
    - [✓] Model to Face access
    - [🔄] Face Region Structure
      - [✓] Region base implementation (LedSpan, center Point)
      - [✓] Center region initialization
      - [✓] Ring collection setup
      - [✓] Edge collection setup
    - [🔄] Model Relationships
      - [✓] Face-to-Region linking
      - [✓] Region-to-LED indexing
      - [ ] LED-to-Point mapping
      - [ ] Point-to-Face assignment
      - [ ] Neighbor relationships
      - [ ] Relationship Validation
        - [ ] Compile-time ModelDef checks
        - [ ] Runtime relationship integrity
        - [ ] Pointer ownership & lifetime
    - [ ] LED Management
      - [✓] LedSpan implementation
      - [ ] Color access patterns
        - [ ] Memory safety validation
        - [ ] Direct LED color assignment
        - [ ] Region fill operations
        - [ ] FastLED array synchronization
        - [ ] FastLED integration

2. Test Cases

- ✓ Basic enum value validation
- ✓ Type consistency checks
- [✓] Metadata validation
- [🔄] Collection access patterns
  - [✓] Model.faces() access
  - [✓] Face.regions() access
  - [✓] Region.leds() access
  - [ ] LED/Point relationships

Next Steps:

A. Complete Model Relationships Testing
- Add tests for all ModelDef relationships:
  - Face -> Regions (center, rings, edges)
  - Region -> LEDs (indices correct)
  - LED -> Point (coordinates match)
  - Point -> Face (assignments valid)
  - Point -> Neighbors (connections valid)
  - Validate pointer lifetimes and ownership
  - Test relationship integrity across operations

B. Validate Collection Access
- Test array_view implementations
- Verify bounds checking
- Test iteration patterns
- Ensure const correctness
- Standardize error handling across collections
- Document collection access patterns

C. Add Feature Implementation
- LED color management
- Prove FastLED integration performance
- Geometric operations
- Neighbor traversal
- Create animation proof-of-concept

## Future Considerations

### Teensy41 Platform Investigation

Current Issues:

- Binary.h macro conflicts with Eigen - ✓ FIXED using ArduinoEigen
- ARM GCC iterator differences from native
- Memory model differences in STL containers

Verified Platform Features:

- std::array with full C++17 features ✓
- Template deduction guides ✓
- Range-based for loops ✓
- Move semantics ✓
- Reference types ✓
- STL containers (vector, initializer_list) ✓
- Memory alignment (alignas) ✓
- array_view implementation works in both environments ✓

