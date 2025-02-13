---
generated: 2025-02-13 18:48
version: 2.8.3
---

# Parameter System Refactoring

## Overview

The parameter system enables scenes to define, validate and access typed values in a safe way, supporting both YAML and manual parameter definition with a clean user interface.

## User Experience Requirements

From Scenes.md, we need to support:

1. YAML Parameter Definition:

```yaml
parameters:
  speed:
    type: ratio
    default: 0.5
    description: "Controls animation speed"
    flags: [clamp]
```

2. Manual Parameter Definition:

```cpp
void setup() override {
    param("speed", "ratio", 0.5f, "clamp");
    float speed = settings["speed"];
}
```

3. Runtime Usage:

```cpp
void tick() override {
    float speed = settings["speed"];        // Clean conversion
    settings["speed"] = 0.5f;              // Clean assignment
    bool enabled = settings["is_enabled"];  // Type safety
}
```

## Architecture & API

### 1. Scene Layer (User Interface)

Namespace: PixelTheater

* Scene
  * setup() - Parameter definition point
  * settings[] - Parameter access proxy
  * tick() - Animation update point

* Settings
  * add_parameter() - Store parameter definition
  * get_value()/set_value() - Value access with validation
  * reset_all() - Reset to defaults

* SettingsProxy
  * operator[] - Returns Parameter proxy
  * Parameter - Single parameter proxy
    * operator T() - Type-safe conversion
    * operator=(T) - Type-safe assignment

### 2. Parameter Layer (Type System)

Namespace: PixelTheater::Params

* ParamDef
  * validate() - Uses handlers for validation
  * apply_flags() - Uses handlers for flags
  * get_metadata() - Parameter info

* ParamValue
  * constructors - Type-locked construction
  * as_<type>() - Uses handlers for conversion
  * can_convert_to() - Uses handlers for type checks

### 3. Handler Layer (Core Logic)

Namespace: PixelTheater::Params::Handlers

* TypeHandler
  * validate() - Type validation
  * can_convert() - Conversion checks
  * get_info() - Type metadata
  * from_string() - String parsing

* RangeHandler
  * validate() - Range validation
  * clamp() - Range clamping
  * wrap() - Range wrapping

* FlagHandler
  * apply() - Flag application
  * has_flag() - Flag checking
  * from_string() - String parsing

* SentinelHandler
  * get_sentinel<T>() - Get sentinel value
  * is_sentinel() - Check sentinel
  * handle_error() - Error handling

### 4. Core Layer (Platform Abstraction)

Namespace: PixelTheater::Core

* Log
  * warning() - Log warnings
  * set_log_function() - For testing

* Math
  * is_nan() - NaN check
  * is_inf() - Inf check

## Type System

Namespace: PixelTheater::Params::Types

```
ParamType                                // Parameter types
├── Numeric                             // Value types
│   ├── ratio                          // [0.0 .. 1.0]
│   ├── signed_ratio                   // [-1.0 .. 1.0]
│   ├── angle                          // [0.0 .. PI]
│   ├── signed_angle                   // [-PI .. PI]
│   ├── range                          // [min .. max] float
│   └── count                          // [min .. max] int
├── Choice                              // Selection types
│   ├── select                         // Named options
│   └── switch_type                    // Boolean
└── Resource                           // Asset types
    ├── palette                        // Color palette
    └── bitmap                         // Image data

ParamFlags                              // Parameter flags
├── NONE                               // No special handling
├── CLAMP                              // Clamp to range
├── WRAP                               // Wrap around range
└── SLEW                               // Rate limited
```

## Error Handling

### Sentinel Values by Type

```
float: 0.0f                            // Invalid float value
int: -1                                // Invalid integer value
bool: false                            // Invalid boolean value
nullptr                                // Invalid pointer/resource
```

### Error Flow

1. Detection: Handlers detect invalid states
2. Reporting: Log::warning() reports issues
3. Propagation: SentinelHandler provides type-safe error values
4. Handling: Scene code can check for sentinel values

## Parameter Definition and Flow

The parameter system supports two paths for defining parameters, both using the same validation chain.

### Definition Sources

1. YAML Definition (Generated at Build)

```yaml
parameters:
  speed:
    type: ratio          # Must match ParamType
    default: 0.5         # Must be valid for type
    flags: [clamp]       # Optional behavior flags
    description: "..."   # Optional metadata
```

2. Manual Definition (Runtime in Scene)

```cpp
void setup() override {
    param("speed", "ratio", 0.5f, "clamp");
}
```

### Parameter Flow

1. Creation Phase
   * YAML → _params.h generation → Scene construction
   * OR Scene::setup() → param() → Settings::add_parameter()
   * Both create ParamDef objects

2. Validation Phase
   * Type validation (is type valid?)
   * Range validation (is default in range?)
   * Flag validation (are flags compatible?)
   * All validation uses handlers

3. Storage Phase
   * ParamDef stored in Settings._params
   * Default value stored in Settings._values
   * Metadata accessible via Settings.get_metadata()

4. Access Phase
   * Scene code: settings["param"]
   * SettingsProxy provides clean interface
   * Values validated on every access
   * Flags applied on every set

### Validation Chain

All validation failures:

1. Log warning
2. Return sentinel
3. Preserve type safety
4. Allow recovery

## File Structure

```
lib/PixelTheater/
├── include/PixelTheater/
│   ├── params/
│   │   ├── handlers/               # Core parameter operations
│   │   │   ├── type_handler.h      # Type system and validation
│   │   │   ├── range_handler.h     # Range operations
│   │   │   ├── flag_handler.h      # Flag operations
│   │   │   └── sentinel_handler.h  # Sentinel value management
│   │   │
│   │   ├── types/                  # Type definitions
│   │   │   ├── param_type.h        # Enum and type info
│   │   │   ├── param_value.h       # Value container
│   │   │   └── param_flags.h       # Flag definitions
│   │   │
│   │   ├── param_def.h            # Parameter definition
│   │   └── param_macros.h         # User-friendly macros
│   │
│   └── settings/                   # Settings system
```

## Benefits

1. Clear namespace hierarchy
2. Consistent dependency direction
3. Platform independence
4. Type safety
5. Clean user interface

## Migration Plan

### Current Status

Implemented:

* SentinelHandler - Complete, tested ✓
* Log system - Complete, tested ✓
* Basic parameter validation ✓
* Platform-independent math wrappers ✓
* Test fixtures for logging ✓
* Namespace organization for handlers ✓
* TypeHandler basic validation ✓
* Type metadata and conversion rules ✓
* RangeHandler float/int wrapping ✓
* Edge case handling for ranges ✓
* Platform-independent wrapping logic ✓
* Range validation with proper error messages ✓
* Correct handling of custom ranges vs default ranges ✓
* Sentinel value handling for all types ✓
* Type validation chain complete ✓
* Flag validation and rules ✓
* Settings validation chain (Type -> Range -> Flag) ✓
* Settings integration tests complete ✓
* YAML parameter loading (via generated headers) ✓
* Manual parameter definition ✓
* Constructor validation moved to TypeHandler ✓
* Flag handling for all parameter types ✓
* Simplified apply_flags implementation ✓
* Added comprehensive flag validation tests ✓

In Progress:

* Scene Integration [NEXT]
* Final Cleanup Tasks [NEXT]:
* * Review param_range.h for unused code
* * Check for any remaining validation duplication
* * Update documentation for flag behavior
* * Consider removing get_sentinel_for_type from ParamDef (now in TypeHandler)

### Phase 1: Core Infrastructure [COMPLETED] ✓

1. Create handlers/ directory structure ✓
2. Implement SentinelHandler ✓
3. Add Log system improvements ✓
4. Create handler test fixtures ✓
5. Establish handler namespace structure ✓

### Phase 2: Type System [COMPLETED] ✓

1. Move type-related code from ParamValue to TypeHandler
   * Extract type checking logic ✓
   * Add type conversion rules ✓
   * Add type metadata ✓
   * Add validation rules ✓
2. Update tests to use TypeHandler ✓
3. Modify ParamValue to delegate to TypeHandler ✓

### Phase 3: Range Operations [COMPLETED] ✓

1. Create RangeHandler
   * Move range validation from ParamDef ✓
   * Move clamp/wrap operations ✓
   * Add range metadata ✓
   * Handle edge cases ✓
   * Platform independence ✓
2. Add RangeHandler tests ✓
3. Update ParamDef to use RangeHandler ✓

### Phase 4: Flag System [COMPLETED] ✓

1. Create FlagHandler
   * Define flag validation rules ✓
   * Handle flag conflicts (CLAMP vs WRAP) ✓
   * Add type-specific flag rules ✓
   * Add warning messages for invalid flags ✓
2. Add FlagHandler tests ✓
3. Update ParamDef to use FlagHandler ✓

### Phase 5: Settings Layer [COMPLETED] ✓

1. Update Settings to use handlers ✓
   * Use TypeHandler for type validation ✓
   * Use RangeHandler for range validation ✓
   * Use FlagHandler for flag validation ✓
2. Add validation chains ✓
   * Type -> Range -> Flag validation order ✓
   * Clear error messages at each step ✓
   * Proper sentinel propagation ✓
3. Update error propagation ✓
   * Warning messages for type errors ✓
   * Warning messages for range errors ✓
   * Warning messages for flag errors ✓
4. Add integration tests ✓
   * Test validation chain ✓
   * Test error messages ✓
   * Test sentinel propagation ✓
   * Test edge cases ✓

### Phase 6: Scene Integration [COMPLETE] ✓

1. Test YAML parameter loading ✓
2. Test manual parameter definition ✓
3. Test parameter inheritance ✓
   * Test base scene parameters ✓
   * Test parameter overrides ✓
   * Test flag preservation ✓
   * Test metadata inheritance ✓
4. Update documentation ✓
   * Add parameter inheritance examples ✓
   * Document validation chain ✓
   * Update Scene class docs ✓

### Phase 7: Platform Testing [COMPLETE]

1. Test on native environment
   * Test parameter validation ✓
   * Test memory management ✓
   * Test error handling ✓
   * Test inheritance chain ✓
2. Test on Teensy
   * Test parameter storage 
   * Test memory constraints
   * Test logging system
   * Test real-time behavior
3. Document platform differences
   * Memory usage
   * Error handling
   * Logging behavior
   * Performance considerations
4. Add platform-specific tests
   * Native-only tests
   * Hardware-only tests
   * Common test suite

### Phase 8: Cleanup [IN PROGRESS]

1. Remove old validation code ✓
2. Update all tests ✓
3. Update documentation [IN PROGRESS]
4. Version bump [PENDING]

Each phase should:

* Have clear success criteria
* Be testable in isolation
* Not break existing code
* Be reversible if issues found

## Platform Constraints

### Hardware Environment (Teensy)

1. Exception Handling
   * No exceptions (disabled in hardware)
   * Use sentinel values instead of throw
   * Log warnings for error conditions

2. Floating Point
   * NaN/Inf checks must be platform-specific
   * Use math_platform.h wrappers
   * Handle invalid floats with sentinels

3. Memory Constraints
   * Avoid dynamic allocation
   * Use constexpr where possible
   * Keep parameter storage compact

4. Logging
   * Use platform-independent Log system
   * Serial output on hardware
   * printf/cout in native

### Handler Requirements

Handlers must:

* Be exception-safe
* Use platform-independent math
* Avoid dynamic memory
* Be constexpr-friendly where possible
* Handle platform differences internally

### Testing Strategy

1. Native Tests
   * Full validation
   * Memory checks
   * Error conditions

2. Hardware Tests
   * Basic functionality
   * Platform-specific features
   * Resource usage
   * Serial logging
