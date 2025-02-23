# PixelTheater Development Plan

Reference docs:
‼️ @model.md : Model structure and data format documentation
‼️ @scene.md : Scene class documentation

## Current Status (Updated March 2024)

### Completed Features ✓

1. Core Model Implementation
- Point system with coordinates and IDs
- LED array access and manipulation
- Face management and indexing
- Collection operations (fill, iterate)
- Model definition integration
- Basic geometric operations

2. LED Groups System
- LedGroup class with non-contiguous LED access
- Named group definitions and construction
- Face-local and global group access
- Group operations (union, intersection, difference)
- FastLED-compatible function wrappers
- Comprehensive test coverage

### In Progress Features 🚧

1. Spatial Query System
- Neighbor data structure defined
- Basic distance calculations
- Need implementation of findNearby()
- Need spatial indexing

### Remaining Features ❌

1. Scene System
- Scene class implementation
- Animation framework
- Transition effects
- Common pattern library

2. Stage Implementation
- Scene management
- Animation timing
- Global effects
- Hardware synchronization

## Next Steps (Prioritized)

### A. Complete Spatial Query System
Priority: HIGH
Estimated time: 2-3 days

1. Core Implementation
```cpp
// Proposed API
class SpatialIndex {
    std::vector<size_t> findNearby(const Point& p, float radius);
    std::vector<size_t> findNearbyFast(size_t led_index, float radius);
    float distanceBetween(size_t led1, size_t led2);
};
```

2. Tasks
- [ ] Implement findNearby() using existing neighbor data
- [ ] Add spatial indexing for performance
- [ ] Create distance-based helper functions
- [ ] Add unit tests
- [ ] Benchmark performance

### B. Scene System Implementation
Priority: MEDIUM
Estimated time: 1 week

1. Core Components
```cpp
class Scene {
    virtual void setup();
    virtual void update(uint32_t ms);
    virtual void enter();
    virtual void exit();
    
    // Animation helpers
    void crossfade(const Scene& other, uint16_t duration_ms);
    void transition(TransitionType type, uint16_t duration_ms);
};
```

2. Tasks
- [ ] Define Scene lifecycle
- [ ] Implement basic animation framework
- [ ] Create common pattern library
- [ ] Add transition effects
- [ ] Document usage patterns

### C. Stage Implementation
Priority: MEDIUM
Estimated time: 3-4 days

1. Core Features
```cpp
class Stage {
    void addScene(Scene* scene);
    void transitionTo(Scene* scene, uint16_t duration_ms);
    void update();
    void setBrightness(uint8_t brightness);
};
```

2. Tasks
- [ ] Complete Stage class implementation
- [ ] Add scene management
- [ ] Implement timing system
- [ ] Add global effects (brightness, color correction)
- [ ] Create example animations

### D. Performance Optimization
Priority: LOW
Estimated time: Ongoing

1. Focus Areas
- [ ] Profile LED group operations
- [ ] Optimize spatial queries
- [ ] Measure memory usage
- [ ] Add benchmarks

## Timeline

Week 1:
- Complete Spatial Query System
- Start Scene implementation

Week 2:
- Complete Scene implementation
- Start Stage implementation

Week 3:
- Complete Stage implementation
- Begin optimization
- Documentation updates

## Questions and Decisions

1. Scene System Design
- How should scenes handle state persistence?
- What common animation patterns should be provided?
- How to handle resource management?

2. Performance Considerations
- Maximum number of concurrent animations?
- Memory constraints for different platforms?
- Update frequency requirements?

Would you like to:
A. Start implementing the Spatial Query System?
B. Begin Scene system design?
C. Add more detail to any section?
