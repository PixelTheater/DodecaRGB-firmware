# DodecaRGB Web Simulator

The DodecaRGB Web Simulator provides a browser-based visualization and testing environment for LED animations using WebGL and WebAssembly. This guide explains the architecture, design patterns, and implementation details to help developers understand and modify the codebase.

![web sim](../images/web-simulator.png:600x)

## Architecture Overview

The web simulator uses a hybrid architecture:

- **C++ Core**: Animation logic and scene definitions are implemented in C++, identical to the firmware code
- **WebAssembly Bridge**: Emscripten compiles C++ code to WebAssembly and creates JavaScript bindings
- **JavaScript UI Layer**: Provides interactive controls and WebGL rendering

This architecture allows sharing code between the firmware and simulator, ensuring animations behave consistently across platforms.

## Code Organization

### C++ Components

- **[`src/web_simulator.cpp`](../src/web_simulator.cpp)**: Main C++ entry point for the simulator
  - Contains the `WebSimulator` class that manages scene execution and parameter handling
  - Implements Emscripten bindings for JavaScript interoperability
  - Defines the `SceneParameter` struct for passing parameter data to the UI

- **`lib/PixelTheater/`**: Core animation framework
  - Identical to the code used in the firmware
  - Scene definitions, animations, and parameter management

### JavaScript Components

- **[`web/js/simulator-ui.js`](../../web/js/simulator-ui.js)**: Main JavaScript UI controller
  - `DodecaSimulator` class handles UI interaction and WebGL rendering
  - Manages parameter controls, camera settings, and animation playback
  - Communicates with WebAssembly module through Emscripten bindings

- **[`web/index.html`](../../web/index.html)**: HTML structure for the simulator interface

## Parameter System

The parameter system allows scenes to expose configurable values to the UI. For comprehensive details on parameter types and usage, see the [Parameters documentation](../PixelTheater/Parameters.md).

### Parameter Schema

1. **Parameter Definition in C++ (`ParamDef` class)**:
   - Defines metadata: name, type, default value, range values
   - Supports various types: ratio, signed_ratio, angle, signed_angle, range, count, select, switch_type
   - Each parameter has validation and transformation rules based on its type

2. **Parameter Values in C++ (`ParamValue` class)**:
   - Stores and manipulates parameter values with appropriate type safety
   - Handles conversions between different value representations

3. **Parameter Representation in WebAssembly Bridge (`SceneParameter` struct)**:

```cpp
struct SceneParameter {
      std::string id;           // Parameter identifier
      std::string label;        // Display name
      std::string controlType;  // UI control ("slider", "checkbox", "select")
      std::string value;        // String representation of value
      std::string type;         // Parameter type from C++ (ratio, count, etc.)
      float min;                // Minimum value for numeric parameters
      float max;                // Maximum value for numeric parameters
      float step;               // Step increment for numeric parameters
      std::vector<std::string> options;  // Options for select parameters
};
```

### Parameter Type Handling

The simulator adapts UI controls based on parameter types as defined in the [Parameters documentation](../PixelTheater/Parameters.md):

- **Numeric Types**:
  - `ratio`, `signed_ratio`: Float values in range [0,1] or [-1,1], displayed with 3 decimal places
  - `angle`, `signed_angle`: Float values in radians, displayed with 3 decimal places
  - `range`: Custom float range, displayed with 3 decimal places
  - `count`: Integer values, displayed as rounded integers

- **Option Types**:
  - `select`: Dropdown selection from predefined options
  - `switch_type`: Boolean value displayed as checkbox

### Step Size Calculation

Step sizes are dynamically calculated based on parameter type:
- For `ratio` and `signed_ratio`: 0.01
- For `angle` and `signed_angle`: PI/100
- For `range`: (max-min)/100
- For `count`: 1

## Emscripten Bindings

The simulator uses Emscripten to create JavaScript bindings for C++ functions:

```cpp
EMSCRIPTEN_BINDINGS(scene_parameters) {
    emscripten::function("getSceneParameters", &get_scene_parameters_wrapper);
    emscripten::function("updateSceneParameter", &update_scene_parameter_wrapper);
    // Additional bindings for simulator control...
}
```

These bindings enable JavaScript to:
1. Get scene parameter definitions
2. Update parameter values
3. Control simulator behavior (change scenes, adjust camera, etc.)

JavaScript accesses these functions through the global `Module` object:

```javascript
// Example of calling a bound function from JavaScript
const parameters = Module.getSceneParameters();
```

## UI Control System

The UI controls are dynamically generated based on parameter definitions:

1. **Parameter Retrieval**: When a scene is selected, the UI calls `Module.getSceneParameters()` to get parameter definitions
2. **Control Generation**: For each parameter, appropriate controls are created based on `controlType` and `type`
3. **Value Storage**: Parameter values are stored in memory to persist across scene changes
4. **Event Handling**: Input changes trigger `handleSceneParameterChange()` which calls `Module.updateSceneParameter()`

### Value Formatting

The UI formats parameter values based on their type (in [`simulator-ui.js`](../../web/js/simulator-ui.js)):

```javascript
const formatValue = (val, paramType) => {
    if (paramType === 'count') {
        return Math.round(val);
    }
    if (paramType === 'ratio' || paramType === 'signed_ratio') {
        return parseFloat(val).toFixed(3);  // 3 decimal places for ratios
    }
    if (paramType === 'angle' || paramType === 'signed_angle') {
        return parseFloat(val).toFixed(3);  // 3 decimal places for angles
    }
    if (paramType === 'range') {
        return parseFloat(val).toFixed(3);  // 3 decimal places for ranges
    }
    return val;
};
```

## Rendering and Display Controls

The simulator provides controls for adjusting the visual representation:

- **WebGL Rendering**: Uses a WebGL context to render the 3D model
- **Visual Settings**: Brightness, LED size, mesh visibility, opacity, atmosphere intensity
- **Animation Updates**: Renders at up to 60fps using `requestAnimationFrame`

## Camera System

The camera system allows users to navigate the 3D space:

- **Manual Rotation**: Click and drag to rotate the model
- **Auto-rotation**: Toggle automatic rotation with speed control
- **Preset Views**: Predefined viewing angles
- **Zoom Levels**: Different distance settings from the model

## Initialization Sequence

The initialization sequence follows these steps:

1. **DOM Ready**: Wait for HTML document to load
2. **WebGL Support Check**: Verify browser supports WebGL2
3. **Module Initialization**:
   - Create WebAssembly module
   - Set up JavaScript callbacks
   - Initialize rendering context
4. **Scene Setup**:
   - Load scene definitions
   - Set default scene
   - Generate UI controls
5. **Event Binding**: Connect UI elements to event handlers
6. **Start Rendering Loop**: Begin animation frame requests

For more information on scene implementation, see the [Scenes documentation](../PixelTheater/Scenes.md).

## Performance Considerations

- **Benchmarking**: Built-in performance monitoring
- **Memory Management**: Proper cleanup of resources when switching scenes
- **FPS Control**: Monitoring and logging of frame rate
- **Floating-Point Precision**: Careful handling of decimal values to maintain animation quality

## Building the Simulator

### Prerequisites

- Emscripten SDK (3.1.0+)
- Build system (Make)

### Build Commands

```bash
# Build the simulator
pio run -e web

# Start development server
pio run -t serve
```

## Troubleshooting

- **Console Logging**: Browser console provides debug information
- **Debug Mode**: Toggle debug mode for additional logging
- **Common Issues**: Browser compatibility, WebGL support, parameter precision

## Contributing

When modifying the web simulator:
1. Test in multiple browsers
2. Update this documentation for architectural changes
3. Follow the project's coding style guidelines
