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
  - Implements C functions exposed via `EMSCRIPTEN_KEEPALIVE` for JavaScript interoperability (e.g., `get_scene_parameters_json`, `update_scene_parameter_string`)
  - Parameter data is primarily exchanged with JavaScript using JSON strings.

- **`lib/PixelTheater/`**: Core animation framework
  - Identical to the code used in the firmware
  - Scene definitions, animations, and parameter management

Note: for logging to the web browser, use the built-in `logInfo()` method in web_platform.

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

3. **Parameter Representation via JSON**:
   - C++ functions (`get_current_scene_metadata_json`, `get_scene_parameters_json`) generate JSON strings containing parameter definitions (ID, label, description, control type, current value, type, min/max/step, options).
   - JavaScript fetches these JSON strings, parses them, and dynamically creates UI controls.
   - When a UI control value changes, JavaScript calls a C++ function (`update_scene_parameter_string`) passing the parameter ID and the new value as strings. C++ then parses the string value based on the parameter's type.
   - **Memory Management**: C++ functions returning JSON strings allocate memory using `malloc`. JavaScript is responsible for calling a corresponding C++ function (`free_string_memory`) to release this memory after parsing the string.

```cpp
// Example C functions exposed to JavaScript:
extern "C" {
  EMSCRIPTEN_KEEPALIVE const char* get_scene_parameters_json();
  EMSCRIPTEN_KEEPALIVE void update_scene_parameter_string(const char* param_id_cstr, const char* value_cstr);
  EMSCRIPTEN_KEEPALIVE void free_string_memory(char* ptr);
  // ... other control functions ...
}
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

The simulator uses Emscripten to expose C++ functionality to JavaScript primarily through C functions marked with `EMSCRIPTEN_KEEPALIVE`. This avoids the overhead and complexity of Embind for this interface.

```cpp
// Example C functions exposed to JavaScript:
extern "C" {
  EMSCRIPTEN_KEEPALIVE bool init_simulator();
  EMSCRIPTEN_KEEPALIVE void change_scene(int sceneIndex);
  EMSCRIPTEN_KEEPALIVE const char* get_scene_parameters_json();
  EMSCRIPTEN_KEEPALIVE void update_scene_parameter_string(const char* param_id_cstr, const char* value_cstr);
  EMSCRIPTEN_KEEPALIVE void free_string_memory(char* ptr);
  // ... other control functions like set_brightness, update_rotation etc. ...
}
```

These functions become available on the global `Module` object in JavaScript, typically accessed via helper functions or directly using `Module._functionName` or `Module.ccall`.

JavaScript accesses these functions like this:

```javascript
// Example of calling a bound function from JavaScript
let paramsPtr = 0;
try {
    paramsPtr = Module._get_scene_parameters_json(); // Call C function
    if (paramsPtr) {
        const paramsJsonString = Module.UTF8ToString(paramsPtr); // Read string from WASM memory
        const parameters = JSON.parse(paramsJsonString);
        // ... process parameters ...
    }
} finally {
    if (paramsPtr) {
        Module._free_string_memory(paramsPtr); // Free the memory allocated by C++
    }
}

// Example using ccall for parameter updates
Module.ccall(
    'update_scene_parameter_string', // C function name
    null,                           // Return type
    ['string', 'string'],           // Argument types
    [paramId, newValueString]       // Arguments
);
```

## UI Control System

The UI controls are dynamically generated based on parameter definitions received as JSON:

1. **Parameter Retrieval**: When a scene is selected, the UI calls `Module._get_scene_parameters_json()` (wrapped in `callModule`) to get parameter definitions as a JSON string.
2. **Control Generation**: For each parameter object in the parsed JSON, appropriate controls are created based on `controlType` and `type` (see `addSceneParameter` in JS).
3. **Value Storage**: Parameter values are stored in JavaScript (`this.sceneParameters`) to persist across scene changes and potentially restore state.
4. **Event Handling**: Input changes trigger `handleSceneParameterChange()`, which calls the C++ function `update_scene_parameter_string` via `Module.ccall`, passing the parameter ID and the new value as strings.

### Value Formatting

The UI formats parameter values for display based on their type (in [`simulator-ui.js`](../../web/js/simulator-ui.js)):

```javascript
// Helper function to format displayed values (from simulator-ui.js)
const formatValue = (valStr, paramType) => {
    const val = parseFloat(valStr); // Convert string value to number for formatting
    if (paramType === 'count') {
        return Math.round(val);
    }
    // For float types, show appropriate precision
    if (paramType === 'ratio' || paramType === 'signed_ratio' ||
        paramType === 'angle' || paramType === 'signed_angle' ||
        paramType === 'range') {
        // Adjust precision based on step or range if needed
        return val.toFixed(3);
    }
    // Fallback for unknown types or boolean strings
    return valStr;
};
```

## Rendering and Display Controls

The simulator provides controls for adjusting the visual representation:

- **WebGL Rendering**: Uses a WebGL context to render the 3D model
- **Visual Settings**: Brightness, LED size, mesh visibility, opacity, atmosphere intensity
- **Animation Updates**: Renders at up to 60fps using `requestAnimationFrame`

## Camera System

The camera system allows users to navigate the 3D space:

- **Free Rotation**: Click and drag to rotate the model in any direction
  - Left/right drag rotates around the Y axis (yaw)
  - Up/down drag rotates around the X axis (pitch)
  - Rotation is consistent regardless of the model's current orientation
  - No rotation limits - the model can be freely rotated in all directions

- **Auto-rotation**: Toggle automatic rotation with speed control
  - Off: No automatic rotation
  - Slow: Gentle rotation around the Y axis
  - Fast: Quicker rotation around the Y axis

- **Zoom Levels**: Different distance settings from the model
  - Close: Detailed view of the model
  - Normal: Standard viewing distance
  - Far: Zoomed out view of the entire model

- **Mouse Wheel**: Adjust zoom level dynamically

- **Double-click**: Reset rotation to the default position

The camera system uses a turntable-style rotation model where the camera position remains fixed while the model rotates. This provides an intuitive interaction model for examining the 3D object from all angles.

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
./build_web.sh

# Start development server
python -m http.server -d web
```

## Build Process & Troubleshooting

The web simulator build relies on Emscripten to compile the C++ core (`PixelTheater`, scenes, `web_simulator.cpp`) into WebAssembly (`.wasm`) and JavaScript glue code (`.js`). The PlatformIO environment (`web`) orchestrates this by typically calling a `Makefile` located in the `web/` directory via the `scripts/web_build.py` script.

### Key Build Flags and Concepts

Understanding some `emcc` (Emscripten Compiler Frontend) flags used during the build is crucial:

*   **`-I<path>`:** Specifies include directories for the C++ compiler (e.g., `-Ilib`, `-Isrc`).
*   **`-DPLATFORM_WEB`, `-DEMSCRIPTEN`:** Defines preprocessor macros used for conditional compilation specific to the web build.
*   **`-std=c++20`:** Specifies the C++ standard.
*   **`-s WASM=1`:** Explicitly enables WebAssembly output.
*   **`-s USE_WEBGL2=1`, `-s FULL_ES3=1`:** Enables WebGL2 support, which is required by the simulator's renderer.
*   **`-s ALLOW_MEMORY_GROWTH=1`:** Allows the WebAssembly heap memory to grow dynamically if needed, preventing crashes if the initial allocation is insufficient. Paired with `-s INITIAL_MEMORY` and `-s MAXIMUM_MEMORY`.
*   **`-s EXPORTED_FUNCTIONS=[...]`:** This is **critical**. It provides a list of C function symbols (prepended with `_`) that should be preserved and made callable from JavaScript. Even if a function is marked with `EMSCRIPTEN_KEEPALIVE` in C++, it often needs to be listed here as well to guarantee it's accessible via `Module._functionName` or `Module.ccall`. **If you add a new C function intended to be called from JavaScript, you MUST add its `_` prefixed name to this list in the build configuration (e.g., `web/Makefile`).**
*   **`-s EXPORTED_RUNTIME_METHODS=[...]`:** Lists Emscripten runtime methods needed by the JS code (e.g., `UTF8ToString`, `ccall`).
*   **`-g`, `-O0`, `-s ASSERTIONS=2`, `-s SAFE_HEAP=1`, `-s GL_DEBUG=1`, `-s STACK_OVERFLOW_CHECK=2`:** Debugging flags. These are useful during development but should typically be changed (`-O2` or `-O3`, assertions disabled) for release builds to improve performance and reduce code size.
*   **`--source-map-base ./`:** Helps browsers map compiled code back to the original source for easier debugging.
*   **`-o "$OUTPUT_DIR/simulator.js"`:** Specifies the output JavaScript file. Emscripten will also generate the corresponding `.wasm` file.

### Development Cycle & Troubleshooting Steps

1.  **Modify Code:** Make changes to C++ (`.cpp`, `.h`) or JavaScript (`.js`, `.css`, `.html`) files.
2.  **Build:** Run `./build_web.sh` in your terminal. This invokes the build process using Emscripten directly.
3.  **Run Server:** Run `python -m http.server -d web` (or your preferred method) from the project root directory. This usually starts a local web server (often on `http://localhost:8000`).
4.  **Test:** Open the simulator URL in your web browser (e.g., `http://localhost:8000`).
5.  **Debug:**
    *   **Browser DevTools are Essential:** Open your browser's developer console (usually F12). Check for errors in the Console tab.
    *   **JavaScript Logging:** In `simulator-ui.js`, use standard browser console methods like `console.log('UI State:', this.someVariable);`, `console.warn('Unexpected value.');`, or `console.error('Failed to process:', error);`. These messages appear directly in the browser console.
    *   **C++ Logging:** Within C++ code (like `WebPlatform` or Scenes), use the platform's logging methods: `logInfo("Processing item %d", index);`, `logWarning("Value out of expected range: %f", val);`, `logError("Initialization failed with code %d", errCode);`. These methods are implemented in `WebPlatform` to call the corresponding JavaScript `console` methods, prefixing the output with `[INFO]`, `[WARN]`, or `[ERR ]`. Standard `printf()` calls from C++ also output to the browser console.
    *   **Check Exported Functions:** If JS fails to call a C++ function, double-check that the function name (with `_` prefix) is listed in the `-s EXPORTED_FUNCTIONS` flag within the build configuration (e.g., `web/Makefile`).
    *   **Hard Refresh:** Browsers cache aggressively. Use **Shift+Cmd+R** (Mac) or **Ctrl+Shift+R** (Windows/Linux) to perform a hard refresh, bypassing the cache.
    *   **Memory Issues:** Look for errors related to memory allocation (`allocate`, `INITIAL_MEMORY`, `Maximum memory size`) in the console. You might need to adjust `-s INITIAL_MEMORY` or `-s MAXIMUM_MEMORY` in the build flags.
    *   **WebGL Errors:** Check the console for WebGL-specific errors. Ensure `-s USE_WEBGL2=1` is set.
    *   **Assertion Failures:** If assertions are enabled (`-s ASSERTIONS=2`), failed C++ `assert()` calls will print messages to the console.
    *   **Build Errors:** Examine the output of `./build_web.sh` carefully for C++ compilation errors (missing includes, syntax errors) or linking errors (missing symbols, often related to exported functions).

### Common Gotchas

*   **Forgetting Exports:** Forgetting to add a new C function to `-s EXPORTED_FUNCTIONS` in the Makefile is a very common cause of "function not found" errors in JavaScript.
*   **Browser Caching:** The browser cache can prevent you from seeing the latest changes. Always do a hard refresh.
*   **Memory Management:** C++ code that returns strings (`const char*`) allocated with `malloc` (like `get_scene_parameters_json`) requires the JavaScript caller to explicitly free that memory using a corresponding C++ function (e.g., `Module._free_string_memory(ptr)`).
*   **Build Configuration:** Ensure the correct include paths (`-I`) and source files are specified in the build configuration (Makefile).
*   **Type Mismatches:** Ensure the types used in JS `ccall` calls match the C function signature exactly.

## Troubleshooting

- **Console Logging**: Browser console provides debug information
- **Debug Mode**: Toggle debug mode for additional logging
- **Common Issues**: Browser compatibility, WebGL support, parameter precision

## Contributing

When modifying the web simulator:
1. Test in multiple browsers
2. Update this documentation for architectural changes
3. Follow the project's coding style guidelines
