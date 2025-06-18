# PixelTheater Easier API Refactoring - Project Plan

## Palette Restructuring (Platform Transparent API)

### Goal Summary

To provide a simple, reliable, and **platform-transparent** way for scenes to use color palettes and utilities. Scenes will interact solely with the `PixelTheater` namespace, which will internally handle platform differences (FastLED on Teensy, C++ emulation/fallbacks elsewhere) without requiring conditional compilation within scene code.

### Core Principles

1.  **Consistent `PixelTheater` Namespace**: All types (`CRGB`, `CHSV`, `CRGBPalette16`) and core functions (`colorFromPalette`, `blend`, etc.) used by scenes reside within and are accessed via the `PixelTheater` namespace.
2.  **Internal Platform Handling**: The *implementation* of `PixelTheater` types and functions will conditionally use FastLED natives (when `PLATFORM_TEENSY`) or C++ fallback/emulation logic.
3.  **API Abstraction**: Scenes include only `PixelTheater` headers, insulating them from underlying FastLED details or fallback implementations.
4.  **Unified Palette Access**: All standard FastLED palettes and project-specific custom palettes will be exposed as `constexpr` or `extern const` constants within the `PixelTheater::Palettes` namespace (e.g., `PixelTheater::Palettes::Rainbow`, `PixelTheater::Palettes::basePalette`).

## Web Simulator Loop Refactoring (JavaScript Driven)

### Goal Summary

To refactor the web simulator's main loop architecture to reliably display FPS and align with standard web development practices. This involves shifting loop control and FPS calculation from C++ to JavaScript.

### Core Principles

1.  **JavaScript Loop Control**: The main animation loop will be driven by JavaScript using `requestAnimationFrame` for optimal browser integration and performance.
2.  **JavaScript Timing & FPS**: JavaScript will be responsible for calculating the time delta between frames (using the timestamp from `requestAnimationFrame`) and calculating the FPS metric.
3.  **Direct DOM Update**: JavaScript will directly update the FPS display element in the HTML DOM.
4.  **C++ as Simulation Engine**: The C++ WASM module will act as a simulation engine, exposing a function to be called by JavaScript each frame to advance the simulation state and trigger rendering.
5.  **Clear Responsibilities**: JavaScript handles browser integration, timing, and UI updates. C++ handles simulation logic and rendering commands. `WebPlatform` becomes purely a rendering interface for the web build.
6.  **Eliminate Problematic Bridge**: Removes the unreliable C++ (`update_ui_fps`) to JS (`_update_ui_fps`) function call bridge.

### Implementation Steps

1.  **Modify `web/js/simulator-ui.js`**:
    *   Remove the current `Module.onRuntimeInitialized` logic that relies on C++'s `emscripten_set_main_loop`.
    *   Initiate a JavaScript-driven loop using `requestAnimationFrame` within the `onModuleReady` function (or equivalent starting point after WASM is loaded).
    *   Implement the `jsAnimationLoop` function:
        *   Calculate `deltaTime` using the timestamp provided by `requestAnimationFrame`.
        *   Calculate average FPS (e.g., update calculation once per second).
        *   Get the `fps-counter` element and update its `textContent`.
        *   Call the new C++ step function (e.g., `_do_simulation_step`) via `Module.ccall` or direct function call (`Module._do_simulation_step()`).
        *   Recursively call `requestAnimationFrame(jsAnimationLoop)` to schedule the next frame.
    *   Remove the JavaScript function `updateUIFps` and its assignment to `window._update_ui_fps` / `Module._update_ui_fps`.

2.  **Modify `src/web_simulator.cpp`**:
    *   Remove the `emscripten_set_main_loop` call from the `main()` function. The `main` function might become empty or just contain initial logging.
    *   Define a new C-style function `EMSCRIPTEN_KEEPALIVE void do_simulation_step()` that contains the single call: `if (g_simulator) { g_simulator->update(); }`.

3.  **Modify `lib/PixelTheater/src/platform/web_platform.cpp`**:
    *   Remove the entire FPS calculation block from the `WebPlatform::show()` method (including time retrieval, frame counting, the `if` block, and related variable initializations like `_last_frame_time`, `_frame_count`).
    *   Remove the C function `update_ui_fps(int fps)`.
    *   Remove the C function `call_js_update_fps(int fps)`.

4.  **Modify `build_web.sh`**:
    *   In the `-s EXPORTED_FUNCTIONS=[...]` list:
        *   Remove `"_update_ui_fps"`.
        *   Remove `"_call_js_update_fps"`.
        *   Keep `"_get_current_time"` for now as it might be used elsewhere (e.g., auto-rotation).
        *   Add `"_do_simulation_step"`.

