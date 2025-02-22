# PixelTheater Development Plan

Reference docs:
‼️ @model.md : Model structure and data format documentation
‼️ @scene.md : Scene class documentation

# Implementation Plan

## A. Core Point Implementation (Already Done ✓)

- Basic Point class with coordinates and IDs is implemented
- Distance calculations working
- Neighbor detection implemented
- Tests passing

## B. Basic Model Structure

1. Create minimal Model class with LED array access:

```cpp
class Model {
    std::array<CRGB, LED_COUNT> _leds;
public:
    CRGB& operator[](size_t i) { return _leds[i]; }
    const CRGB& operator[](size_t i) const { return _leds[i]; }
};
```

2. Write tests verifying:
- Construction with LED_COUNT
- Basic LED access and modification
- Bounds checking behavior

## C. Point Collection Access

1. Add Point storage and access:

```cpp
class Model {
    std::array<Point, LED_COUNT> _points;
public:
    const Point& point(size_t i) const { return _points[i]; }
};
```

2. Write tests for:
- Point access matches LED indexing
- Coordinate access through points
- Point-to-LED relationships

## D. Basic Face Implementation

1. Create Face class with local LED indexing:

```cpp
class Face {
    Model& _model;
    size_t _offset;
    size_t _count;
public:
    CRGB& led(size_t local_idx);
    size_t led_offset() const;
};
```

2. Write tests covering:
- Face construction with offset/count
- Local-to-global LED index conversion
- LED access through face

## E. Face Collection in Model

1. Add face management to Model:

```cpp
class Model {
    std::array<Face, FACE_COUNT> _faces;
public:
    Face& face(size_t i);
    size_t face_count() const;
};
```

2. Test cases for:
- Face count verification
- Face access and LED relationships
- Face-local operations

## F. Model Definition Integration

1. Update Model to use ModelDefinition:

```cpp
class Model {
    using Definition = ModelDefinition<LED_COUNT, FACE_COUNT>;
    const Definition& _def;
public:
    Model(const Definition& def);
};
```

2. Tests verifying:
- Construction from definition
- Point data initialization
- Face data initialization

## Next Steps After Core Implementation

A. Add iterator support for collections
B. Implement fill() and other bulk operations
C. Add neighbor relationship support
D. Implement geometric helper functions

Would you like me to start with the implementation and tests for any particular step? I recommend starting with step B (Basic Model Structure) since it provides the foundation for everything else.
