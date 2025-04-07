# PixelTheater Easier API Refactoring - Project Plan

This document outlines the steps required to implement the API simplification refactoring as detailed in `docs/easier_api_refactoring.md`. **All steps involving code changes should be followed by running the test suite (`~/.platformio/penv/bin/pio test -e native`) to ensure no regressions.**

## Design Overview

This refactoring introduces a `Theater` facade class to simplify top-level API usage. Initialization is done via platform-specific methods (e.g., `useFastLEDPlatform<ModelDef>`). It decouples `Scene` implementations from specific `ModelDef` templates by introducing `IModel` and `ILedBuffer` interfaces. 

**Geometry Access:** Scenes access model geometry via `scene.model()`, which returns a `const IModel&`. The `IModel` interface provides direct indexed access methods (`point(i)`, `face(i)`) and count methods (`pointCount()`, `faceCount()`). Bounds checking uses clamping or returns dummy objects. Iteration over geometry is done using the LED index (`for(i=0; i<ledCount()...)`).

Concrete wrapper classes (`ModelWrapper`, `LedBufferWrapper`) implement these interfaces internally within `Theater`. Scene authors interact with a non-templated `Scene` base class, accessing LEDs (via `leds[i]` proxy or `led(i)` helper), Model geometry (via `model().point(i)` etc.), and Platform utilities (via direct `Scene` helpers like `millis()`).

See `docs/easier_api_refactoring.md` for the original goals and the final Target API Summary.

## Phase 1: Core Infrastructure (Interfaces & Wrappers - Simplified Model Access)

**Goal:** Establish the interfaces (`ILedBuffer`, simplified `IModel`), concrete wrappers (`LedBufferWrapper`, `ModelWrapper`), and ensure testability using existing fixtures.

1. **Define Interfaces:**
    * [X] **Task:** Verify/Update `ILedBuffer` interface file (`lib/.../core/iled_buffer.h`). Define `led(i)`, `ledCount()`. Ensure bounds checks use clamping/dummy return.
    * [X] **Task:** Create/Update `IModel` interface file (`lib/.../core/imodel.h`). Define virtual methods `point(i)`, `face(i)`, `pointCount()`, `faceCount()`. Ensure necessary includes (`Point`, `Face`).

2. **Implement `Face` Move Semantics:** 
    * [X] **Task:** Add explicit move/copy constructors/assignment to `Face` class.
    * [X] **Task:** Provide explicit `Face()` default constructor.
    * [X] **Testing:** Native tests pass. **Run tests.**

3. **Create Interface Wrappers:**
    * [X] **Task:** Create/Update `LedBufferWrapper : public ILedBuffer` (`lib/.../core/led_buffer_wrapper.h`). Implement `led(i)`, `ledCount()`. Ensure bounds checks use clamping/dummy return.
    * [X] **Task:** Create/Update `template<typename TModelDef> class ModelWrapper : public IModel` (`lib/.../core/model_wrapper.h`). Implement the *simplified* `IModel` interface, delegating to `concrete_model_->points[i]` etc. Implement clamping/dummy returns for bounds checks.
    * [X] **Testing:** Create/Update `test/test_native/core/test_interface_wrappers.cpp`. Test `ILedBuffer` access. Test `IModel` access via `model_if->point(i)`, `model_if->face(i)`, `model_if->pointCount()`, etc. **Run tests.**

4. **Refactor `Scene` Base Class:**
    * [X] **Task:** Change `Scene` to be non-templated.
    * [X] **Task:** Add `protected` members: `model_ptr`, `leds_ptr`, `platform_ptr`, `_tick_count`.
    * [X] **Task:** Add `protected: void connect(...)`.
    * [X] **Task:** Remove direct geometry helpers. Add `public: const IModel& model() const;`. 
    * [X] **Task:** Ensure `LedsProxy leds;` member works.
    * [X] **Task:** Adapt `param_schema.h`, stub/comment out `Stage`-related code.
    * [X] **Testing:** Deferred to Task 11.

5. **Implement Scene Base Helpers & Test:**
    * [X] **Task:** Add/verify public helpers for LEDs (`led`, `ledCount`) and Utilities (`millis`, `random*`, `log*`). Ensure no exceptions.
    * [X] **Task:** Add/verify `Platform` interface utility methods and implementations.
    * [X] **Task:** Ensure `param()`/`meta()`/`settings` work.
    * [X] **Testing:** Create/Update `test_scene_helpers.cpp`. Test LED/Utility/model() helpers. **Run tests.**

6. **Consolidated Include:**
    * [X] **Task:** Create/update `PixelTheater.h`.
    * [X] **Testing:** Verified compilation.

## Phase 2: Theater Implementation & Integration

**Goal:** Implement the `Theater` facade and integrate it, ensuring scenes can be added and run.

7. **Implement `Theater` Class Structure:**
    * [X] **Task:** Create `Theater` class (`theater.h`, `theater.cpp`). Define members: `std::unique_ptr<Platform> platform_`, `std::unique_ptr<IModel> model_`, `std::unique_ptr<ILedBuffer> leds_`, `std::vector<std::unique_ptr<Scene>> scenes_`, `Scene* current_scene_ = nullptr;`.
    * [X] **Testing:** Create `test/test_native/test_theater.cpp`. Write tests for basic construction/destruction. **Run tests.**

8. **Implement `Theater::useXPlatform` Methods:**
    * [X] **Task:** Implement public platform-specific methods like `template<typename TModelDef> void useNativePlatform(size_t num_leds)`. (Implemented Native variant).
    * [X] **Task:** These methods create the corresponding `Platform` type and call a private `template<typename TModelDef, typename TPlatform> void internal_prepare(std::unique_ptr<TPlatform> platform)` method.
    * [X] **Task:** The `internal_prepare` method stores the platform, instantiates the concrete `Model<TModelDef>`, creates and stores the `ModelWrapper` (as `model_`) and `LedBufferWrapper` (as `leds_`) unique_ptrs.
    * [X] **Testing:** Add tests in `test_theater.cpp` to verify `useNativePlatform` successfully calls `internal_prepare` and creates the required internal objects (platform, model wrapper, leds wrapper). Use `BasicPentagonModel` and appropriate args. **Run tests.**

9. **Implement `Theater::addScene` & Scene Management:**
    * [X] **Task:** Implement `template<typename SceneType> void addScene()`. Check initialization, create scene, call `connect()`, store pointer, handle first scene.
    * [X] **Task:** Implement `start()`, `nextScene()`, `previousScene()`, `update()` (call `current_scene_->tick()` and `platform_->show()`).
    * [X] **Task:** Implement scene info/access methods: `scene(index)`, `scenes()`, `currentScene()`, `sceneCount()`.
    * [X] **Testing:** Add tests in `test_theater.cpp` for adding scenes, switching scenes, checking current scene, accessing scenes by index, and calling update. **Run tests.**

10. **Refactor `main.cpp`:**
    * [X] **Task:** Remove manual `Model`/`Stage`/`Platform`. Instantiate `Theater`. Call the appropriate `theater.useXPlatform<ModelDef>(...)` method (e.g., `useFastLEDPlatform`). Replace `stage->addScene` with `theater.addScene<Type>()`. Replace `stage->update` with `theater.update()`. Adapt scene switching and parameter setting using `theater.scene(index)`.
    * [X] **Testing:** Verify `main.cpp` compiles. Native tests verify library integrity. Hardware testing needed for full validation. (Teensy build successful). **Run tests.**

## Phase 3: Refactor Scenes & Documentation

**Goal:** Adapt existing scenes to the new API and update documentation.

11. **Refactor Example Scenes (One by One):**
    * [X] **Task:** Refactor test scenes (`TestScene`, `LEDTestScene`, `MetadataTestScene`, `ParamTestScene`, etc.) in `test_scene.cpp` / `test_scene_parameters.cpp`.
    * [X] **Task:** Apply refactoring: remove template, inherit `Scene`, use `PixelTheater.h`, use helpers (`leds[i]`, `model().point(i)`, `random8`, etc.).
    * [X] **Testing:** Adapt related tests (`test_scene.cpp`, `test_scene_parameters.cpp`) using manual setup or new fixture. **Run tests.** (Native tests pass).
    * [X] **Repeat** for `blob_scene.h`. (Compiled for teensy41)
    * [X] **Repeat** for `xyz_scanner`. (Compiled for teensy41)
    * [X] **Repeat** for `wandering_particles`. (Compiled for teensy41)

12. **Update Documentation:**
    * [ ] **Task:** Create `docs/PixelTheater/SceneAuthorGuide.md` using the target API content previously drafted.
    * [ ] **Task:** Update `docs/PixelTheater/Scenes.md` to be a higher-level overview linking to the new guide.
    * [ ] **Task:** Review/update `docs/PixelTheater/README.md` (architecture diagram/text).
    * [ ] **Task:** Update examples in docs (`README.md`, etc.) to use `Theater`.

13. **Final Review and Cleanup:**
    * [ ] **Testing:** Perform thorough testing on hardware with multiple scenes.
    * [ ] **Task:** Review code for clarity, comments, style, null checks, error handling (`addScene` before initialization).
    * [ ] **Task:** Remove dead code (old `Stage`? `scene.h` template remnants?).
    * [ ] **Task:** Archive or delete `docs/easier_api_refactoring.md`.
    * [ ] **Testing:** **Run final tests.**

## Target Scene API Summary

*(This section summarizes the intended API available to Scene Authors within their `Scene` subclass)*

**Core Setup:**
* `#include "PixelTheater.h"`
* `using namespace PixelTheater;` (Recommended)
* `using namespace PixelTheater::Constants;` (Recommended)
* `class MyScene : public PixelTheater::Scene { ... };`

**Lifecycle:**
* `virtual void setup();`
* `virtual void tick();`

**Parameters & Metadata:**
* `param(...)` (Multiple overloads as per `Parameters.md`)
* `meta(key, value)`
* `settings[key]` (Read access, returns `std::any` wrapper)
* `name()` (Returns title metadata)

**LED Access:**
* `CRGB& led(size_t index);` (Bounds-checked)
* `const CRGB& led(size_t index) const;`
* `std::span<CRGB> leds();` (Or similar span/wrapper)
* `const std::span<const CRGB> leds() const;`
* `size_t ledCount() const;`

**Model Geometry Access:**
* `const Point& point(size_t index) const;` (Bounds-checked)
* `const Face& face(size_t index) const;` (Bounds-checked)
* `size_t faceCount() const;`

**Timing Utilities:**
* `float deltaTime() const;`
* `uint32_t millis() const;`
* `uint32_t tickCount() const;` (Frame counter for current activation)

**Math/Random Utilities:**
* `uint8_t random8();`
* `uint16_t random16();`
* `uint32_t random(uint32_t max = RAND_MAX);`
* `uint32_t random(uint32_t min, uint32_t max);`
* `float randomFloat();` (0.0 to 1.0)
* `float randomFloat(float max);` (0.0 to max)
* `float randomFloat(float min, float max);`
* *(Access to global utilities like `map`, `blend`, `fadeToBlackBy` via `using namespace`)*

**Logging Utilities:**
* `logInfo(format, ...);`
* `logWarning(format, ...);`
* `logError(format, ...);`

---

**Notes:**

* Error handling: No exceptions. Clamping/dummy returns for invalid access, logging for errors.
* **Decisions Log:**
    * Use `Theater` facade.
    * Use non-templated `Scene`.
    * Use `ILedBuffer`/`IModel` interfaces.
    * Scene access: `leds[i]`/`led(i)`, Utils helpers, `model()` method.
    * `model()` returns `const IModel&`.
    * `IModel` provides indexed access: `point(i)`, `face(i)`, `pointCount()`, `faceCount()`. No proxies.
    * `useXPlatform<ModelDef>()` / `addScene<SceneType>()`.

## Phase 4: Web Platform Integration (Native-Testable Parts First)

**Goal:** Adapt the C++ components for web integration, prioritizing changes testable in the native environment before addressing web-specific build and runtime issues. **Run native tests (`~/.platformio/penv/bin/pio test -e native`) after each C++ refactoring step.**

*   **Sub-Phase 4.1: C++ Core Changes (Native Testable)**
    *   14. **Implement `WebPlatform` Utilities (Stubs for Native Testing):**
        *   [X] **Task:** Add *declarations* for missing `Platform` virtual methods (`deltaTime`, `millis`, `random*`, `log*`) in `WebPlatform` (`lib/PixelTheater/platform/web_platform.h`).
        *   [X] **Task:** Add *dummy implementations* or basic native equivalents for these methods in `lib/PixelTheater/src/platform/web_platform.cpp`.
        *   [X] **Task:** Ensure `WebPlatform::getLEDs()` and `WebPlatform::initializeWithModel<ModelDef>()` method signatures exist in the header with minimal stub implementations in the `.cpp` file.
        *   [X] **Native Testing:** Updated tests in `test/test_web/test_web_platform.cpp` verify existence and native compilation of stubs. **Native tests passed.**
    *   15. **Implement `Theater::useWebPlatform`:**
        *   [X] **Task:** Added `template<typename TModelDef> void Theater::useWebPlatform()` method declaration to `theater.h` and implementation (using stubs) to `theater.cpp` (guarded by `#ifdef`).
        *   [X] **Task:** Implementation creates `WebPlatform`, calls its (stubbed) `initializeWithModel`, and passes to `internal_prepare`.
        *   [X] **Native Testing:** Added guarded test case (`test_native/test_theater.cpp`) verifying compilation and basic calls for `useWebPlatform` in web environment (excluded from native run). **Native tests passed.**
    *   16. **Refactor `WebSimulator` Class (C++ Logic):**
        *   [ ] **Task:** Replace `Stage`, `Platform`, `Model` members in `WebSimulator` (`src/web_simulator.cpp`) with `std::unique_ptr<Theater> theater`.
        *   [ ] **Task:** Update `WebSimulator::initialize()` C++ logic to instantiate `Theater` and call `theater->useWebPlatform<ModelDef>(...)`.
        *   [ ] **Task:** Update `WebSimulator::update()` C++ logic to call `theater->update()`.
        *   [ ] **Task:** Add `bool Theater::setScene(size_t index)` method declaration to `theater.h` and basic implementation (e.g., setting `current_scene_`) to `theater.cpp`. Update `WebSimulator::setScene(int sceneIndex)` C++ logic to use `theater->setScene(index)`.
        *   [ ] **Task:** Update `addScene` calls within `WebSimulator::initialize`'s C++ logic to use `theater->addScene<SceneType>()`.
        *   [ ] **Task:** Update parameter methods (`getSceneParameters`, `updateSceneParameter`) and their Embind wrappers C++ logic to use `theater->currentScene()`/`theater->scene(index)` and the associated parameter schema/settings storage.
        *   [ ] **Task:** Update C++ methods bound to JS for platform features (brightness, rotation, etc.) to access the platform via `theater->platform()` and `dynamic_cast<WebPlatform*>(...)`.
        *   [ ] **Native Testing:** Ensure `web_simulator.cpp` compiles cleanly within the *native* test environment build process. Expand tests in `test/test_web/test_main.cpp` (or a new `test_web_simulator.cpp`) to test the *refactored C++ logic* of `WebSimulator`. Verify that:
            *   `initialize` creates the `Theater` instance and calls `useWebPlatform`.
            *   `update` calls `theater->update`.
            *   `addScene` calls `theater->addScene`.
            *   `setScene` calls `theater->setScene`.
            *   Parameter methods correctly interact with `theater->currentScene()->parameter_schema()` and `_settings_storage`.
            *   Platform feature methods correctly retrieve the platform via `theater->platform()` and attempt `dynamic_cast`.
            *   Use the stubbed `WebPlatform` provided by `Theater` for these native tests. **Run native tests.** (Testing the Emscripten bindings and JS interaction will happen in Sub-Phase 4.2).

*   **Sub-Phase 4.2: Web Build & Runtime Integration**
    *   17. **Update Build System:**
        *   [ ] **Task:** Add `$(PIXELTHEATER_DIR)/src/theater.cpp` to `SOURCES` in `web/Makefile`.
        *   [ ] **Task:** Add `$(PIXELTHEATER_DIR)/src/platform/web_platform.cpp` to `SOURCES` in `web/Makefile`.
        *   [ ] **Task:** Review and update `EXPORTED_FUNCTIONS` and `EMSCRIPTEN_BINDINGS` in `web_simulator.cpp` and the `Makefile` flags to ensure all necessary C++ functions/bindings called by JavaScript (`simulator-ui.js`) are still correctly exported and functional after refactoring `WebSimulator`.
        *   [ ] **Testing:** Attempt to run the web build script (`build_web.sh` or `python build_docs.py`). Address compilation and linking errors.
    *   18. **Finalize `WebPlatform` Utilities:**
        *   [ ] **Task:** Replace dummy implementations in `lib/PixelTheater/src/platform/web_platform.cpp` with actual Emscripten/browser API calls (`performance.now()`, `Math.random()`, `console.log`, etc.) guarded by `#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)`.
        *   [ ] **Task:** Ensure the implementations of `initializeWithModel` and `getLEDs` correctly interact with the WebGL setup and buffer management within the `#ifdef` guards.
        *   [ ] **Testing:** Re-run the web build script. Browser testing deferred.
    *   19. **Web Simulator Testing & Documentation:**
        *   [ ] **Testing:** Perform thorough testing of the web simulator in a browser (rendering, scenes, parameters, controls, console errors).
        *   [ ] **Task:** Update `docs/guides/web-simulator.md` to reflect the use of `Theater`.
        *   [ ] **Task:** Update any relevant diagrams or code snippets in documentation.

## Phase 5: Final Review and Cleanup

**Goal:** Consolidate changes, remove obsolete code, and perform final verification across all platforms.

*   20. **Final Review and Cleanup:**
    *   [ ] **Testing:** Perform thorough testing on **all platforms** (native, hardware, web).
    *   [ ] **Task:** Review code across the project (core lib, `main.cpp`, `web_simulator.cpp`, tests) for clarity, comments, style, null checks, error handling.
    *   [ ] **Task:** Remove `Stage` class (`stage.h`, `stage.cpp`) and any remaining unused code related to the old architecture.
    *   [ ] **Task:** Archive or delete `docs/easier_api_refactoring.md`.
    *   [ ] **Testing:** **Run final tests on all platforms.** Build the web simulator and documentation (`python build_docs.py`).
