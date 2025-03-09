---
category: Development
generated: 2025-02-13 18:48
version: 2.8.3
---

# Parameters

## What are Parameters For?

Parameters allow for more flexibility and easier configuration. They:

- Define an interface contract between the scene and the Director
- Enable preset support for saving and loading configurations
- Provide type safety and range validation
- Allow remote control and real-time adjustment
- Make scenes more reusable and configurable

> Parameters define constraints and behaviors that help prevent errors and
> make animations more robust. They also enable features like presets and
> remote control that would be difficult to implement with raw variables.

## Defining Parameters

Parameters are defined in the `setup()` method of your Scene class. This approach offers a clear and maintainable way to configure your scenes directly in code.

```cpp
// Example Scene Parameter Definition
void setup() override {
    // Define parameters with name, type, default value, and optional flags
    param("speed", "ratio", 0.5f, "clamp", "Controls animation speed");
    param("brightness", "ratio", 0.8f, "wrap", "Overall LED brightness");
    param("count", "count", 0, 100, 50, "clamp", "Number of particles");
    param("enabled", "switch", true, "", "Enable or disable the effect");
    
    // Selection parameter with options
    param("pattern", "select", {"sphere", "fountain", "cascade"}, "sphere", "", "Animation pattern");
}
```

In this example:
- Each parameter is defined with a name, type, and default value
- Additional arguments can include ranges (for applicable types), flags, and descriptions
- The parameters become available through the scene's `settings` object

## Parameter Types and Ranges

Parameters serve as the foundation of our animation control system, providing a bridge between the raw code and user-friendly controls. They're designed to be both intuitive for creators and safe for runtime execution. To achieve this, parameters are strongly-typed values that control how animations behave.

Different parameter types allow you to define various kinds of controls, from simple numeric ranges to selection menus.

For example, instead of defining float ranges manually, use semantic types like `ratio`
for values like brightness or opacity. The ranges will be handled automatically.

### Semantic Types (Fixed Ranges)

- ratio: 0.0 .. 1.0 (normalized value)
  - For intensities, sizes, and other normalized values
- signed_ratio: -1.0 .. 1.0 (bidirectional normalized value)
  - For speeds, factors, and bidirectional controls
- angle: 0.0 .. PI (radians)
  - For rotations and angular measurements
- signed_angle: -PI .. PI (radians)
  - For relative angles and bidirectional rotations

### Basic Types (Custom Ranges)

- range: min .. max (float, requires range)
- count: min .. max (integer, requires range)

### Choice Types

- switch: Boolean true/false (a "checkbox" setting)
- select: Named options that map to integer indices

## Parameter Flags

Parameter flags control how values are handled when they change. They help create
smoother animations and prevent unwanted behavior. Multiple flags can be combined
to achieve the desired effect.

Flags control value handling based on parameter type:

- Numeric types (ratio, signed_ratio, angle, signed_angle, range):
  • `clamp`: Values are limited to their defined range
  • `wrap`: Values wrap around their defined range

- Integer types (count, select):
  • `clamp`: Values are limited to their integer range
  • `wrap`: Values wrap around their integer range

- Non-numeric types (switch):
  No flags supported - values are validated directly

Example:

```cpp
param("speed", "ratio", 0.5f, "clamp", "Animation speed");
```

Note: Flags are validated at runtime. Invalid flag combinations
(like using both clamp and wrap) or unsupported flags for a type will result
in validation errors.

## Parameter API Reference

### param() Method Overloads

All parameters are defined in setup() using the following param() method variations:

1. Basic Parameter Definition

```cpp
// param(name, type, default_value, flags = "", description = "")
param("speed", "ratio", 0.5f);                    // Basic definition
param("speed", "ratio", 0.5f, "clamp");          // With flags
param("speed", "ratio", 0.5f, "clamp", "Controls animation speed"); // With description
```

2. Range Parameters

```cpp
// param(name, type, min, max, default_value, flags = "", description = "")
param("count", "count", 0, 100, 50);             // Integer range
param("scale", "range", 0.0f, 10.0f, 5.0f);      // Float range
param("count", "count", 0, 100, 50, "", "Number of particles"); // With description, no flags
```

3. Select Parameters

```cpp
// param(name, type, values, default_value, flags = "", description = "")
// With string array
param("mode", "select", {"sphere", "fountain", "cascade"}, "sphere");

// With value mapping
param("direction", "select", {
    {"forward", 1},
    {"reverse", -1},
    {"oscillate", 0}
}, "forward", "", "Movement direction");
```

4. Switch Parameters

```cpp
// param(name, type, default_value, flags = "", description = "")
param("enabled", "switch", true, "", "Enable or disable the effect");
```

### Quick Reference Table

| Type          | Range          | Example Usage |
|---------------|----------------|---------------|
| ratio         | 0.0 .. 1.0    | `param("brightness", "ratio", 0.5f)` |
| signed_ratio  | -1.0 .. 1.0   | `param("speed", "signed_ratio", 0.0f)` |
| angle         | 0.0 .. PI     | `param("rotation", "angle", 0.0f)` |
| signed_angle  | -PI .. PI     | `param("direction", "signed_angle", 0.0f)` |
| range         | custom float   | `param("scale", "range", 0.0f, 10.0f, 5.0f)` |
| count         | custom int     | `param("particles", "count", 0, 100, 50)` |
| switch        | true/false     | `param("enabled", "switch", true)` |
| select        | named options  | `param("mode", "select", {"a", "b"}, "a")` |

### Common Flags

- `clamp`: Limit values to defined range
- `wrap`: Wrap values around defined range

Example combining multiple parameters:

```cpp
void setup() override {
    // Basic parameters
    param("speed", "ratio", 0.5f, "clamp");
    param("direction", "signed_ratio", 0.0f);
    
    // Range parameters
    param("particles", "count", 0, 100, 50);
    param("scale", "range", 0.0f, 10.0f, 5.0f);
    
    // Selection and switches
    param("mode", "select", {"sphere", "fountain"}, "sphere");
    param("enabled", "switch", true);
}
```

> Note: All parameter definitions are validated at runtime. Invalid combinations
> or values will trigger validation errors.

## Parameter Inspection and Runtime Access

Parameters can be inspected at runtime to discover a scene's capabilities and interface. This is useful for:

- Building dynamic user interfaces
- Serializing and deserializing scene configurations
- Remote control and monitoring

### Accessing Parameter Values

The most common operation is accessing parameter values:

```cpp
// Get parameter values with automatic type conversion
float speed = settings["speed"];
int count = settings["count"];
bool enabled = settings["enabled"];

// Set parameter values
settings["speed"] = 0.8f;
settings["count"] = 42;
settings["enabled"] = false;
```

### Accessing Parameter Metadata

You can access parameter metadata directly through the settings object:

```cpp
// Access parameter metadata
std::string type = settings["speed"].type();
float min = settings["speed"].min();
float max = settings["speed"].max();
float default_val = settings["speed"].default_value();
std::string description = settings["speed"].description();
ParamFlags flags = settings["speed"].flags();

// Check parameter properties
bool has_range = settings["speed"].has_range();
bool is_select = settings["speed"].is_select_type();
bool is_clamped = settings["speed"].has_flag(Flags::CLAMP);

// For select parameters
if (settings["mode"].is_select_type()) {
    auto options = settings["mode"].options();
}
```

### Parameter Collection Access

You can get a list of all parameter names:

```cpp
// Get all parameter names
auto param_names = settings.names();

// Check if a parameter exists
if (settings.has_parameter("speed")) {
    // Use the parameter
}
```

### Parameter Schema

For UI generation or serialization, you can get the complete parameter schema:

```cpp
// Get complete parameter schema
auto schema = scene->parameter_schema();

// Convert to JSON for web interfaces
std::string json = scene->parameter_schema_json();
```

Parameters are designed to be immutable after definition. The parameter schema (names, types, ranges) should not change during runtime, though their values can be modified through the settings interface.

## Parameter Inheritance

Scenes can inherit parameters from base scenes to promote code reuse and maintain consistent behavior:

```cpp
// Base scene with common parameters
class BaseEffect : public Scene<ModelDef> {
    void setup() override {
        param("speed", "ratio", 0.5f, "clamp", "Animation speed");
        param("enabled", "switch", true, "", "Enable effect");
    }
};

// Derived scene inherits and extends
class DerivedEffect : public BaseEffect {
    void setup() override {
        // First call base setup to inherit parameters
        BaseEffect::setup();
        
        // Then add or override parameters
        param("size", "ratio", 1.0f, "", "Effect size");  // Add new parameter
    }
};
```

When overriding the `setup()` method in a derived class, be sure to call the base class's `setup()` method first to inherit its parameters.

## Internal Parameter Representation

Internally, parameters are represented by the `ParamDef` structure, which stores all metadata about a parameter:

```cpp
struct ParamDef {
    std::string name;
    ParamType type;
    std::string type_name;  // Human-readable type name
    
    // Unified storage for range values
    float min_value;
    float max_value;
    
    // Default values (only one is used based on type)
    float default_float;
    int default_int;
    bool default_bool;
    
    std::vector<std::string> options;  // For select type
    
    ParamFlags flags;
    std::string description;
    
    // Methods for value validation and transformation
    bool validate_value(const ParamValue& value) const;
    ParamValue apply_flags(const ParamValue& value) const;
    
    // Helper methods for inspection
    bool has_flag(ParamFlags flag) const;
    bool has_range() const;
    bool is_select_type() const;
    
    // Get appropriate default value based on type
    ParamValue get_default_value() const;
    
    // Factory methods for creating parameters
    static ParamDef create_ratio(const std::string& name, float default_val, 
                                ParamFlags flags, const std::string& description);
    static ParamDef create_count(const std::string& name, int min, int max, int default_val, 
                                ParamFlags flags, const std::string& description);
    // Other factory methods...
};
```

This structure is used both for defining parameters and for inspecting them at runtime. It provides a unified representation that can be easily serialized for UI generation or other purposes.

## Removed Features

The following features have been removed from the parameter system:

1. **YAML-based Parameter Configuration**: Parameters must now be defined directly in code using the `param()` method in the `setup()` function.

2. **Resource Parameter Types**: The `palette` and `bitmap` parameter types have been removed. Resource management will be handled by a separate system in the future.

3. **Constexpr Requirements**: The parameter system no longer requires constexpr compatibility, allowing for more flexible string handling with std::string.

4. **Generated Code**: There is no longer any code generation step for parameters. All parameters are defined and processed at runtime.

## Refactoring Plan

The parameter system refactoring is being implemented in these focused steps:

1. **✅ Simplify ParamDef Structure**
   - Replace const char* with std::string
   - Unify storage for range values and defaults
   - Add helper methods for type checking and value access
   - Remove resource type support

2. **✅ Enhance Parameter Proxy**
   - Add metadata access methods (type(), min(), max(), etc.)
   - Add helper methods for type checking (has_range(), is_select_type())
   - Implement flag checking methods

3. **✅ Fix String Handling in Log Functions**
   - Ensure proper handling of std::string in log messages
   - Update code to use c_str() when passing strings to variadic functions

4. **✅ Update Settings Implementation**
   - Replace macro usage with factory methods
   - Update field access to use new unified fields
   - Fix validation and default value handling

5. **✅ Fix Scene.h Metadata References**
   - Update ParamDef::Metadata references to use ParamDef directly
   - Ensure proper inheritance of parameters in derived scenes

6. **✅ Implement Parameter Names Collection**
   - Add names() method to Settings/SettingsProxy
   - Ensure proper iteration support

7. **✅ Refactor Settings.cpp**
   - Replace nested switch statement with cleaner if-else structure
   - Improve code organization and readability
   - Ensure proper error handling

8. **✅ Create Parameter Schema**
   - Define schema structure for serialization
   - Implement parameter_schema() method in Scene
   - Add JSON serialization support

9. **✅ Update Parameter Definition**
   - Simplify param() method overloads
   - Remove YAML-based parameter loading
   - Ensure proper handling of empty flags with descriptions

10. **✅ Clean Up Legacy Code**
    - Remove unused YAML generation code
    - Remove resource parameter handling
    - Update documentation

11. **✅ Updated parameter definition with simplified method overloads**

Next immediate steps:

1. 🔜 Update documentation to reflect the new parameter system
   - Update Scenes.md to remove YAML references
   - Update README.md to reflect the new parameter system
   - Add examples of using the new parameter system

Remaining challenges:

1. Scene inheritance and constructor issues in test files
2. Clean up python parameter generation code, tests and related docs in /util
