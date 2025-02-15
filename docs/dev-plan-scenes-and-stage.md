# Development Plan for Scenes and Stage

## Overview

Build core Scene and Stage functionality with clean syntax and model integration.

## Phase 1: Model Access Layer

Goal: Clean access to model constants and data

- Create Model namespace with constants
- Test fixture for basic model data
- Model data access patterns
- Test model data access

## Phase 2: LED Array Access

Goal: Clean LED array syntax with bounds checking

- Stage::leds[] array access
- Bounds checking and sentinel values
- FastLED array management
- Test LED array operations

## Phase 3: Basic Stage

Goal: Stage initialization and LED control

- Stage construction with model data
- LED array assignment
- Basic LED operations
- Test stage operations

## Phase 4: Scene Framework

Goal: Scene lifecycle and LED access

- Scene base class
- Stage reference in scenes
- Scene setup/tick lifecycle
- Test scene operations

## Phase 5: Hardware Integration

Goal: Validate on device

- FastLED integration
- Main loop timing
- Basic test scene
- Hardware validation

## Phase 6: Region Support

Goal: Clean region syntax and operations

- LedSpan implementation
- Chainable operations
- Region access through stage
- Test region operations

## Phase 7: Points Integration

Goal: Integrate existing points.cpp functionality

- Import existing Point class and data
- Create test fixture that matches points.cpp LED count
- Add point access methods to Stage
- Keep points.cpp unchanged initially
- Test point queries through Stage

## Phase 8: Model/Points Bridge 

Goal: Create clean interface between Model and Points

- Add point lookup to Model namespace
- Bridge between Model indices and Points indices
- Ensure Region operations work with Points data
- Test point-based operations
- Maintain points.cpp compatibility

## Future Work

- Python generator will eventually:
  1. Generate model.h with constants and regions
  2. Generate points data in same format as points.cpp
  3. Combine both into unified model interface
  4. Replace manual points.cpp
  5. Support multiple model types

## Key Principles

1. Clean, chainable syntax
2. Always check headers and includes when updating code, including dependencies and proper separation of implentation and interface
3. Use consistent access patterns
4. Bounds checking everywhere
5. Test-driven development
6. Hardware validation later
7. Don't break working points code
8. Keep points system independent
9. Plan for future integration

## Success Criteria

1. Our API reads naturally and follows the syntax defined in Scenes.md and Stage.md
2. LED operations are safe and consistent
3. Region operations are intuitive and consistent
4. Tests verify behavior and are located in the appropriate test file
5. Hardware performs as expected
6. Points system remains functional
7. Model and Points systems cooperate cleanly
8. Existing point queries still work
9. Path clear for future generator work
