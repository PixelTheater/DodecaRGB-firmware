#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
// This file is only used in web builds

#include <emscripten.h>
#include <emscripten/html5.h>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <cstdio>
#include <cmath> // Needed for M_PI
#include <algorithm> // Needed for std::find_if
#include <cstdlib> // Needed for malloc, free
#include <cstring> // Needed for strcpy (use strncpy for safety)
#include <iomanip> // For escaping potentially

// <<< REMOVED EXTRA INCLUDE BASE PLATFORM >>>
// #include "PixelTheater/platform/platform.h"

#include "PixelTheater/theater.h"
// REMOVED: #include "PixelTheater/platform/web_platform.h" // Should be included via theater.h now
// #include "PixelTheater/model/model.h" // Not needed directly anymore
#include "PixelTheater/core/log.h" // For logging
#include "PixelTheater/params/param_value.h" // For ParamValue
#include "PixelTheater/params/param_schema.h" // For SceneParameterSchema
#include "PixelTheater/scene.h" // Correct path for Scene base class pointer
#include "benchmark.h"
#include "scenes/test_scene/test_scene.h"
#include "scenes/blobs/blob_scene.h"
#include "scenes/wandering_particles/wandering_particles_scene.h"
#include "scenes/xyz_scanner/xyz_scanner_scene.h"
#include "scenes/boids/boids_scene.h"
#include <emscripten/bind.h>

// Include the model definition - we'll use an include guard to prevent multiple definitions
#ifndef DODECARGBV2_MODEL_INCLUDED
#define DODECARGBV2_MODEL_INCLUDED
#include "models/DodecaRGBv2/model.h" // Include the generated model
#endif

// Define global debug flag needed by the library
bool g_debug_mode = false;

// Define the model type we're using - use the correct namespace
using ModelDef = PixelTheater::Models::DodecaRGBv2;

// Add this struct definition after other includes but before the WebSimulator class
struct SceneParameter {
    std::string id;
    std::string label;
    std::string controlType;  // "slider", "checkbox", "select"
    std::string value;        // String representation of the value
    std::string type;         // Parameter type from C++ (e.g. "count", "ratio", etc.)
    float min = 0.0f;         // For numeric parameters
    float max = 1.0f;         // For numeric parameters
    float step = 0.01f;       // For numeric parameters
    std::vector<std::string> options;  // For select parameters
};

// C-compatible struct for parameter data
// IMPORTANT: Keep layout simple, C-compatible types. No std::string etc.
// We will handle strings via char pointers.
struct CSceneParameter {
    char* id;           // Pointer to string in WASM memory
    char* label;        // Pointer to string in WASM memory
    char* controlType;  // Pointer to string in WASM memory
    char* value;        // Pointer to string in WASM memory
    char* type;         // Pointer to string in WASM memory
    float min;
    float max;
    float step;
    // NOTE: Skipping 'options' for simplicity in this C API first attempt
};

// --- ADDED: JSON Escape Helper (copied/adapted from param_schema.cpp) ---
std::string escape_json_helper(const std::string& s) {
    std::ostringstream o;
    for (char c : s) {
        switch (c) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if ('\x00' <= c && c <= '\x1f') {
                    // Control characters need unicode escaping
                    o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(c));
                } else {
                    o << c;
                }
        }
    }
    return o.str();
}
// --- End JSON Escape Helper ---

// Create a WebSimulator class to encapsulate all the functionality
class WebSimulator {
public: // Made public for easier C access
    // Member variables
    std::unique_ptr<PixelTheater::Theater> theater;
    int current_scene = 0; // Still potentially useful for UI state?
    int frame_count = 0;
    
public:
    // Constructor
    WebSimulator() {
        PixelTheater::Log::info("Creating WebSimulator instance...");
    }
    
    // Initialize the simulator
    bool initialize() {
        try {
            // Enable benchmarking
            Benchmark::enabled = true;
            
            // Initialize the Theater instance
            if (!theater) {
                theater = std::make_unique<PixelTheater::Theater>();
                // Call useWebPlatform to initialize platform, model, leds
                theater->useWebPlatform<ModelDef>(); // Uses template definition from theater.h
                
                // Apply initial settings (Platform access through Theater)
                if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                    auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform); // Use PixelTheater::WebPlatform
                    if (web_platform) {
                         web_platform->setBrightness(200); // Example initial brightness
                         web_platform->setZoomLevel(1); 
                         PixelTheater::Log::info("Initial platform settings applied.");
                    } else {
                         PixelTheater::Log::warning("Could not cast platform to WebPlatform in initialize");
                    }
                } else {
                    PixelTheater::Log::error("Theater platform pointer is null after useWebPlatform");
                    return false; // Cannot proceed without a platform
                }

                PixelTheater::Log::info("Theater initialized successfully");
            }
            
            // Add scenes TO THE THEATER
            PixelTheater::Log::info("Adding scenes to Theater...");
            
            // Add all scenes using Theater::addScene
            theater->addScene<Scenes::TestScene>();
            theater->addScene<Scenes::BlobScene>();
            theater->addScene<Scenes::WanderingParticlesScene>();
            theater->addScene<Scenes::XYZScannerScene>();
            theater->addScene<Scenes::BoidsScene>();
            if (theater->sceneCount() == 0) { // CORRECT: Use sceneCount()
                PixelTheater::Log::error("No scenes were added to the theater!");
                return false;
            }
            PixelTheater::Log::info("%d scenes added.", theater->sceneCount()); // Use info
            
            // Set the default scene (first scene) using Theater::setScene
            bool success = theater->setScene(0); // CORRECT: Use setScene
            if (success) {
                current_scene = 0;
                 PixelTheater::Log::info("Initial scene set via Theater: %s", 
                                   theater->currentScene() ? theater->currentScene()->name().c_str() : "Unknown");
            } else {
                PixelTheater::Log::error("Failed to set initial scene in Theater");
                return false;
            }
             
            // Reset benchmark data
            BENCHMARK_RESET();
            
            return true;
        } catch (const std::exception& e) {
            PixelTheater::Log::error("Error during initialization: %s", e.what());
            return false;
        }
    }
    
    // Main update function called every frame
    void update() {
        if (!theater) return;
        
        try {
            frame_count++;
            
            // Update and render the current scene via Theater
            BENCHMARK_START("update");
            theater->update(); // CORRECT: Call theater update
            BENCHMARK_END();
            
        } catch (const std::exception& e) {
            PixelTheater::Log::error("Error in main loop: %s", e.what());
        } catch (...) {
            PixelTheater::Log::error("Unknown error in main loop");
        }
    }
    
    // --- Scene Management --- 
    void setScene(int sceneIndex) {
        if (!theater) {
            PixelTheater::Log::error("Theater not initialized in setScene");
            return;
        }
        
        PixelTheater::Log::info("Scene change requested to index: %d", sceneIndex);
        
        // Convert int to size_t for Theater::setScene
        if (sceneIndex < 0) {
            PixelTheater::Log::warning("Invalid scene index (negative): %d", sceneIndex);
            return;
        }
        size_t index = static_cast<size_t>(sceneIndex);

        // Delegate scene change to Theater
        bool success = theater->setScene(index); // CORRECT: Use setScene

        if (success) {
            current_scene = sceneIndex;
            PixelTheater::Log::info("Theater successfully changed scene to index: %zu", index);
        } else {
            PixelTheater::Log::error("Theater failed to set scene to index: %zu", index);
        }
    }
    
    // Get the number of available scenes
    int getSceneCount() {
        return theater ? static_cast<int>(theater->sceneCount()) : 0; // CORRECT: Use sceneCount()
    }

    // Get a pointer to a specific scene (used by C interface)
    PixelTheater::Scene* getScene(int scene_index) { 
        if (!theater) return nullptr;
        if (scene_index < 0 || static_cast<size_t>(scene_index) >= theater->sceneCount()) { // CORRECT: Use sceneCount()
            return nullptr;
        }
        return &theater->scene(static_cast<size_t>(scene_index)); 
    }
    
    // --- Platform Interaction (Brightness, Rotation, Zoom, etc.) --- 
    void setBrightness(float brightness) {
        if (theater) {
            if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* webPlatform = dynamic_cast<PixelTheater::WebPlatform*>(platform); // Use PixelTheater::WebPlatform
                if (webPlatform) {
                    webPlatform->setBrightness(brightness);
                } else {
                    PixelTheater::Log::warning("WebSimulator::setBrightness: Failed to cast Platform to WebPlatform.");
                }
            } else {
                 PixelTheater::Log::warning("WebSimulator::setBrightness: Theater has no platform.");
            }
        }
    }
    
    float getBrightness() {
        if (theater && theater->platform()) { // CORRECT: Use platform()
            auto* platform = dynamic_cast<PixelTheater::WebPlatform*>(theater->platform()); // Use PixelTheater::WebPlatform
             if (platform) {
                return platform->getBrightness();
             } else {
                 PixelTheater::Log::warning("WebSimulator::getBrightness: Failed to cast Platform to WebPlatform.");
             }
        }
        return 0.0f; // Default or error value
    }
    
    void updateRotation(float delta_x, float delta_y) {
        if (theater) {
            if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform); // Use PixelTheater::WebPlatform
                if (web_platform) {
                    web_platform->updateRotation(-delta_x, -delta_y); 
                } else {
                    PixelTheater::Log::warning("Platform is not a WebPlatform in updateRotation");
                }
            } else {
                PixelTheater::Log::warning("Platform not initialized for rotation update");
            }
        } else {
            PixelTheater::Log::warning("Theater not initialized for rotation update");
        }
    }
    
    void resetRotation() {
        if (theater) {
            if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform); // Use PixelTheater::WebPlatform
                if (web_platform) {
                    web_platform->resetRotation();
                } else {
                    PixelTheater::Log::warning("Platform is not a WebPlatform in resetRotation");
                }
            } else {
                PixelTheater::Log::warning("Platform not initialized for rotation reset");
            }
        } else {
            PixelTheater::Log::warning("Theater not initialized for rotation reset");
        }
    }
    
    void setAutoRotation(bool enabled, float speed) {
        if (theater) {
            if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform); // Use PixelTheater::WebPlatform
                if (web_platform) {
                    web_platform->setAutoRotation(enabled, speed);
                }
            }
        }
    }
    
    void setZoomLevel(int zoom_level) {
        if (theater) {
            if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform); // Use PixelTheater::WebPlatform
                if (web_platform) {
                    web_platform->setZoomLevel(zoom_level);
                }
            }
        }
    }
    
    // --- Debugging & Info --- 
    void showBenchmarkReport() {
        static int last_frame_count = 0;
        static double last_time = emscripten_get_now() / 1000.0;
        static double fps = 60.0; 
        
        double current_time = emscripten_get_now() / 1000.0;
        double elapsed = current_time - last_time;
        
        if (elapsed > 0.5) { 
            int frame_diff = frame_count - last_frame_count;
            fps = frame_diff / elapsed;
            
            PixelTheater::Log::info("FPS: %.2f", fps);
            BENCHMARK_REPORT();
            
            last_frame_count = frame_count;
            last_time = current_time;
        }
    }
    
    void toggleDebugMode() {
        g_debug_mode = !g_debug_mode;
        PixelTheater::Log::info("Debug mode: %s", (g_debug_mode ? "ON" : "OFF"));
    }
    
    // Get LED count from the platform
    int getLEDCount() const {
        if (theater && theater->platform()) { // CORRECT: Use platform()
             auto* platform = dynamic_cast<PixelTheater::WebPlatform*>(theater->platform()); // Use PixelTheater::WebPlatform
             if(platform) return platform->getNumLEDs();
        }
        return 0;
    }
    
    // Get FPS (Placeholder)
    float getFPS() const {
        return 60.0f; // Default
    }

    // --- Scene Parameter Interaction --- 
    std::vector<SceneParameter> getSceneParameters() {
        // <<< ADDED LOG >>>
        PixelTheater::Log::info("***** C++ WebSimulator::getSceneParameters ENTRY POINT *****");
        // <<< END ADDED LOG >>>

        std::vector<SceneParameter> result;
        if (!theater) return result;
        
        auto* scene = theater->currentScene();
        if (!scene) {
             PixelTheater::Log::warning("getSceneParameters: No current scene in Theater.");
             return result;
        }
        
        try {
            auto schema = scene->parameter_schema();
            PixelTheater::Log::info("C++ getSceneParameters: Found %zu parameters for scene '%s'", 
                                  schema.parameters.size(), scene->name().c_str());
            
            for (const auto& param : schema.parameters) {
                SceneParameter p;
                p.id = param.name;
                p.label = param.name;
                p.type = param.type;
                
                auto calculateStepSize = [](const auto& param) -> float {
                    if (param.type == "ratio" || param.type == "signed_ratio") return 0.01f;
                    if (param.type == "angle" || param.type == "signed_angle") return M_PI / 100.0f;
                    if (param.type == "range") return (param.max_value != param.min_value) ? (param.max_value - param.min_value) / 100.0f : 0.01f;
                    if (param.type == "count") return 1.0f;
                    return 0.01f;
                };
                
                if (param.type == "switch") {
                    p.controlType = "checkbox";
                    bool value = scene->settings[param.name]; 
                    p.value = value ? "true" : "false";
                    PixelTheater::Log::info("  Param %s (switch): %s", param.name.c_str(), p.value.c_str());
                } 
                else if (param.type == "select") {
                    p.controlType = "select";
                    p.value = "TODO: Fix select access"; // Placeholder
                    p.options = param.options;
                    PixelTheater::Log::info("  Param %s (select): %s", param.name.c_str(), p.value.c_str());
                } 
                else { // Numeric types
                    p.controlType = "slider";
                    if (param.type == "count") {
                        int intValue = scene->settings[param.name]; 
                        if (intValue == 0 && param.default_int != 0) { 
                             intValue = param.default_int;
                             PixelTheater::Log::info("  Param %s (count): using default %d", param.name.c_str(), intValue);
                        }
                        p.value = std::to_string(intValue);
                        PixelTheater::Log::info("  Param %s (count): %s", param.name.c_str(), p.value.c_str());
                    } else { 
                        float floatValue = scene->settings[param.name]; 
                        if (floatValue == 0.0f && param.default_float != 0.0f) { 
                             floatValue = param.default_float;
                             PixelTheater::Log::info("  Param %s (%s): using default %f", param.name.c_str(), param.type.c_str(), floatValue);
                         }
                        std::stringstream ss;
                        ss.precision(6);
                        ss << std::fixed << floatValue;
                        p.value = ss.str();
                        PixelTheater::Log::info("  Param %s (%s): %s", param.name.c_str(), param.type.c_str(), p.value.c_str());
                    }
                    
                    p.min = param.min_value;
                    p.max = param.max_value;
                    p.step = calculateStepSize(param);
                }
                
                result.push_back(p);
            }
        } catch (const std::exception& e) {
            PixelTheater::Log::error("Error getting scene parameters: %s", e.what());
        }
        
        return result;
    }
    
    void updateSceneParameter(std::string param_id, std::string value) {
        if (!theater) return;
        auto* scene = theater->currentScene();
        if (!scene) {
            PixelTheater::Log::warning("updateSceneParameter: No current scene in Theater.");
            return;
        }
        
        auto& settings = scene->settings;
        
        if (!settings.has_parameter(param_id)) { 
            PixelTheater::Log::warning("Parameter not found in scene settings: %s", param_id.c_str());
            return;
        }
        
        try {
            auto schema = scene->parameter_schema();
            auto it = std::find_if(schema.parameters.begin(), schema.parameters.end(),
                [&param_id](const auto& param) { return param.name == param_id; });
            
            if (it == schema.parameters.end()) {
                PixelTheater::Log::warning("Parameter not found in schema: %s", param_id.c_str());
                return;
            }
            
            if (it->type == "switch") {
                settings[param_id] = (value == "true");
            } else if (it->type == "select") {
                 PixelTheater::Log::warning("TODO: Fix select parameter update for %s", param_id.c_str());
            } else if (it->type == "count") {
                 settings[param_id] = std::stoi(value); 
            } else { 
                 settings[param_id] = std::stof(value);
            }
            
            // Log the update attempt
            PixelTheater::Log::info("Updated parameter %s to %s", param_id.c_str(), value.c_str());

        } catch (const std::invalid_argument& ia) {
            PixelTheater::Log::error("Invalid argument for stoi/stof on param %s: %s", param_id.c_str(), ia.what());
        } catch (const std::out_of_range& oor) {
            PixelTheater::Log::error("Out of range for stoi/stof on param %s: %s", param_id.c_str(), oor.what());
        } catch (const std::exception& e) {
            PixelTheater::Log::error("Error updating parameter %s: %s", param_id.c_str(), e.what());
        }
    }

    // --- Appearance Settings --- 
    void setLEDSize(float size) {
        if (theater) {
            if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform); // Use PixelTheater::WebPlatform
                if (web_platform) {
                    web_platform->setLEDSize(size);
                }
            }
        }
    }
    
    float getLEDSize() const {
        if (theater && theater->platform()) { // CORRECT: Use platform()
            auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(theater->platform()); // Use PixelTheater::WebPlatform
            if (web_platform) {
                return web_platform->getLEDSize();
            }
        }
        return 0.0f;
    }
    
    void setAtmosphereIntensity(float intensity) {
        if (theater) {
             if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform); // Use PixelTheater::WebPlatform
                if (web_platform) {
                    web_platform->setAtmosphereIntensity(intensity);
                } else {
                    PixelTheater::Log::warning("Platform not a WebPlatform in setAtmosphereIntensity");
                }
             } else {
                 PixelTheater::Log::warning("Platform not initialized for atmosphere setting");
             }
        }
    }
    
    float getAtmosphereIntensity() const {
        if (theater && theater->platform()) { // CORRECT: Use platform()
             auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(theater->platform()); // Use PixelTheater::WebPlatform
             if (web_platform) {
                return web_platform->getAtmosphereIntensity();
            }
        }
        return PixelTheater::WebPlatform::DEFAULT_ATMOSPHERE_INTENSITY;
    }
    
    void setShowMesh(bool show) {
        if (theater) {
            if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform); // Use PixelTheater::WebPlatform
                if (web_platform) {
                    web_platform->setShowMesh(show);
                     PixelTheater::Log::info("Set mesh visibility: %s", (show ? "ON" : "OFF"));
                }
            }
        }
    }
    
    bool getShowMesh() const {
        if (theater && theater->platform()) { // CORRECT: Use platform()
            auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(theater->platform()); // Use PixelTheater::WebPlatform
            if (web_platform) {
                return web_platform->getShowMesh();
            }
        }
        return false;
    }
    
    void setMeshOpacity(float opacity) {
        if (theater) {
             if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform); // Use PixelTheater::WebPlatform
                if (web_platform) {
                    web_platform->setMeshOpacity(opacity);
                     PixelTheater::Log::info("Set mesh opacity: %.2f", opacity);
                }
            }
        }
    }
    
    float getMeshOpacity() const {
        if (theater && theater->platform()) { // CORRECT: Use platform()
             auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(theater->platform()); // Use PixelTheater::WebPlatform
             if (web_platform) {
                return web_platform->getMeshOpacity();
            }
        }
        return 0.3f; // Default value
    }
    
    // Needs to be public to be called from C function
    void onCanvasResize(int width, int height) {
        if (theater && theater->platform()) { // CORRECT: Use platform()
             auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(theater->platform()); // Use PixelTheater::WebPlatform
             if (web_platform) {
                 web_platform->onCanvasResize(width, height);
             }
        }
    }
};

// --- Global Instance & Main Loop --- 

// Create a global WebSimulator instance (managed by JS lifecycle)
static std::unique_ptr<WebSimulator> g_simulator;

// Main function - sets up Emscripten main loop
int main() {
    PixelTheater::Log::info("main() called. Waiting for init_simulator()...");
    emscripten_set_main_loop([]() {
        if (g_simulator) { g_simulator->update(); }
    }, 0, 1); 
    return 0;
}

// Helper function to escape JSON strings (Keep)
// std::string escape_json_string(const std::string& s) { ... } // Assumed exists

extern "C" {

EMSCRIPTEN_KEEPALIVE
bool init_simulator() {
    PixelTheater::Log::info("-----> C function init_simulator() called. <-----");
    if (g_simulator) {
        PixelTheater::Log::warning("Simulator already initialized.");
        return true; 
    }
    try {
        g_simulator = std::make_unique<WebSimulator>();
        bool success = g_simulator->initialize();
        if (success) {
            PixelTheater::Log::info("Simulator initialized successfully.");
        } else {
             PixelTheater::Log::error("Simulator initialization failed.");
        }
        PixelTheater::Log::info("-----> C function init_simulator() returning: %s <-----", success ? "true" : "false");
        return success;
    } catch (const std::exception& e) {
        PixelTheater::Log::error("Exception during simulator initialization: %s", e.what());
        g_simulator.reset(); 
        PixelTheater::Log::info("-----> C function init_simulator() returning: false (exception) <-----");
        return false;
    } catch (...) {
        PixelTheater::Log::error("Unknown exception during simulator initialization.");
        g_simulator.reset();
        PixelTheater::Log::info("-----> C function init_simulator() returning: false (unknown exception) <-----");
        return false;
    }
}

// update_simulator is called by the main loop setup in main()
// EMSCRIPTEN_KEEPALIVE
// void update_simulator() { ... }

EMSCRIPTEN_KEEPALIVE
void change_scene(int sceneIndex) {
    if (g_simulator) {
        g_simulator->setScene(sceneIndex);
    } else {
        PixelTheater::Log::error("change_scene called before simulator initialized.");
    }
}

EMSCRIPTEN_KEEPALIVE
int get_num_scenes() { // NEW NAME to match JS
    return g_simulator ? g_simulator->getSceneCount() : 0;
}

// --- REVISED get_current_scene_metadata_json (No goto) ---
EMSCRIPTEN_KEEPALIVE
const char* get_current_scene_metadata_json() {
    const char* default_empty_json = "{}";
    char* allocated_json_str = nullptr;

    // Helper lambda to allocate and return default JSON
    auto allocate_default = [&]() -> char* {
        char* default_str = (char*)malloc(strlen(default_empty_json) + 1);
        if (!default_str) {
            PixelTheater::Log::error("Failed to allocate memory for default empty JSON string!");
            return (char*)"{}"; // Last resort static literal
        }
        strcpy(default_str, default_empty_json);
        return default_str;
    };

    // Initial checks
    if (!g_simulator || !g_simulator->theater) {
        PixelTheater::Log::error("get_current_scene_metadata_json: Simulator not initialized!");
        return allocate_default(); // Early return
    }

    PixelTheater::Scene* scene = g_simulator->theater->currentScene();
    if (!scene) {
        PixelTheater::Log::warning("get_current_scene_metadata_json: No current scene in Theater.");
        return allocate_default(); // Early return
    }

    try {
        std::ostringstream json_stream;
        json_stream << "{";
        json_stream << "\"name\":\"" << escape_json_helper(scene->name()) << "\",";
        json_stream << "\"description\":\"" << escape_json_helper(scene->description()) << "\",";
        json_stream << "\"version\":\"" << escape_json_helper(scene->version()) << "\",";
        json_stream << "\"author\":\"" << escape_json_helper(scene->author()) << "\""; 
        json_stream << "}";

        std::string json_string = json_stream.str();
        PixelTheater::Log::info("Generated Metadata JSON for '%s': %s", scene->name().c_str(), json_string.c_str());

        // Allocate memory and copy
        allocated_json_str = (char*)malloc(json_string.length() + 1);
        if (!allocated_json_str) {
            PixelTheater::Log::error("Failed to allocate memory for metadata JSON string!");
            return allocate_default(); // Early return on allocation failure
        }
        strcpy(allocated_json_str, json_string.c_str());
        return allocated_json_str; // Return successfully allocated string

    } catch (const std::exception& e) {
        PixelTheater::Log::error("Error creating metadata JSON: %s", e.what());
        return allocate_default(); // Early return on exception
    } catch (...) {
         PixelTheater::Log::error("Unknown error creating metadata JSON");
        return allocate_default(); // Early return on unknown exception
    }
}

EMSCRIPTEN_KEEPALIVE
void set_brightness(float brightness) { 
    if (g_simulator) {
        g_simulator->setBrightness(brightness);
    } else {
        PixelTheater::Log::error("set_brightness called before simulator initialized.");
    }
}

EMSCRIPTEN_KEEPALIVE
float get_brightness() { 
    return g_simulator ? g_simulator->getBrightness() : 0.0f;
}

EMSCRIPTEN_KEEPALIVE
void update_rotation(float delta_x, float delta_y) {
    if (g_simulator) {
        g_simulator->updateRotation(delta_x, delta_y);
    } else {
         PixelTheater::Log::error("update_rotation called before simulator initialized.");
    }
}

EMSCRIPTEN_KEEPALIVE
void reset_rotation() {
    if (g_simulator) {
        g_simulator->resetRotation();
    } else {
         PixelTheater::Log::error("reset_rotation called before simulator initialized.");
    }
}

EMSCRIPTEN_KEEPALIVE
void set_auto_rotation(bool enabled, float speed) {
    if (g_simulator) {
        g_simulator->setAutoRotation(enabled, speed);
    } else {
         PixelTheater::Log::error("set_auto_rotation called before simulator initialized.");
    }
}

EMSCRIPTEN_KEEPALIVE
void set_zoom_level(int zoom_level) {
    if (g_simulator) {
        g_simulator->setZoomLevel(zoom_level);
    } else {
         PixelTheater::Log::error("set_zoom_level called before simulator initialized.");
    }
}

EMSCRIPTEN_KEEPALIVE
void show_benchmark_report() {
    if (g_simulator) {
        g_simulator->showBenchmarkReport();
    } else {
         PixelTheater::Log::error("show_benchmark_report called before simulator initialized.");
    }
}

EMSCRIPTEN_KEEPALIVE
void toggle_debug_mode() {
    if (g_simulator) {
        g_simulator->toggleDebugMode();
    } else {
        PixelTheater::Log::error("toggle_debug_mode called before simulator initialized.");
    }
}

EMSCRIPTEN_KEEPALIVE
int get_led_count() {
    return g_simulator ? g_simulator->getLEDCount() : 0;
}

EMSCRIPTEN_KEEPALIVE
float get_fps() {
    return g_simulator ? g_simulator->getFPS() : 0.0f;
}

EMSCRIPTEN_KEEPALIVE
void set_led_size(float size) {
    if (g_simulator) {
        g_simulator->setLEDSize(size);
    } else {
        PixelTheater::Log::error("set_led_size called before simulator initialized.");
    }
}

EMSCRIPTEN_KEEPALIVE
float get_led_size() {
    return g_simulator ? g_simulator->getLEDSize() : 0.0f;
}

EMSCRIPTEN_KEEPALIVE
void set_atmosphere_intensity(float intensity) {
    if (g_simulator) {
        g_simulator->setAtmosphereIntensity(intensity);
    } else {
        PixelTheater::Log::error("set_atmosphere_intensity called before simulator initialized.");
    }
}

EMSCRIPTEN_KEEPALIVE
float get_atmosphere_intensity() {
    return g_simulator ? g_simulator->getAtmosphereIntensity() : PixelTheater::WebPlatform::DEFAULT_ATMOSPHERE_INTENSITY;
}

EMSCRIPTEN_KEEPALIVE
void set_show_mesh(bool show) {
    if (g_simulator) {
        g_simulator->setShowMesh(show);
    } else {
        PixelTheater::Log::error("set_show_mesh called before simulator initialized.");
    }
}

EMSCRIPTEN_KEEPALIVE
bool get_show_mesh() {
    return g_simulator ? g_simulator->getShowMesh() : false;
}

EMSCRIPTEN_KEEPALIVE
void set_mesh_opacity(float opacity) {
    if (g_simulator) {
        g_simulator->setMeshOpacity(opacity);
    } else {
        PixelTheater::Log::error("set_mesh_opacity called before simulator initialized.");
    }
}

EMSCRIPTEN_KEEPALIVE
float get_mesh_opacity() {
    return g_simulator ? g_simulator->getMeshOpacity() : 0.3f;
}

EMSCRIPTEN_KEEPALIVE
void log_message(const char* message) {
    PixelTheater::Log::info("[JS] %s", message);
}

// NEW C function to handle canvas resize events from JS
EMSCRIPTEN_KEEPALIVE
void resizeCanvas(int width, int height) { // Function name matches export
    if (g_simulator && g_simulator->theater) { 
        if (auto* platform = g_simulator->theater->platform()) { 
            auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform); // Use PixelTheater::WebPlatform
            if (web_platform) {
                web_platform->onCanvasResize(width, height); // Call the method
                PixelTheater::Log::info("Called WebPlatform::onCanvasResize(%d, %d)", width, height);
            } else {
                PixelTheater::Log::error("resizeCanvas: Platform is not a WebPlatform.");
            }
        } else {
            PixelTheater::Log::error("resizeCanvas: Theater has no platform.");
        }
    } else {
        PixelTheater::Log::error("resizeCanvas called before simulator/theater initialized.");
    }
}

// Helper to allocate and copy string for C API (Keep for potential future use)
char* allocateAndCopyString(const std::string& str) {
    if (str.empty()) {
        return nullptr; // Return null if string is empty
    }
    // Allocate memory (+1 for null terminator)
    char* cstr = (char*)malloc(str.length() + 1);
    if (!cstr) {
         PixelTheater::Log::error("C_API: Failed to malloc string buffer!");
         return nullptr; // Allocation failed
    }
    // Copy string content
    strncpy(cstr, str.c_str(), str.length());
    cstr[str.length()] = '\0'; // Ensure null termination
    return cstr;
}

// --- REMOVED C API TEST FUNCTION ---
// --- END REMOVED TEST FUNCTION ---

// Original C API function (Commented out for test)
/*
EMSCRIPTEN_KEEPALIVE
CSceneParameter* get_scene_parameters_c_api(int* out_count) {
    // ... original implementation ...
}
*/

// Free function (Keep but won't be called in this test)
EMSCRIPTEN_KEEPALIVE
void free_scene_parameters_c_api(CSceneParameter* params_array, int count) { /* ... */ }

EMSCRIPTEN_KEEPALIVE
void free_string_memory(char* ptr) {
    if (ptr) {
        // Optional: Add a log to confirm freeing
        // printf(">>> C++ free_string_memory freeing ptr %p\n", ptr);
        free(ptr);
    }
}

EMSCRIPTEN_KEEPALIVE
const char* get_scene_parameters_json() {
    if (!g_simulator) {
        PixelTheater::Log::error("get_scene_parameters_json: Simulator not initialized!");
        return nullptr;
    }
    auto* scene = g_simulator->theater->currentScene();
    if (!scene) {
        PixelTheater::Log::warning("get_scene_parameters_json: No current scene in Theater.");
        return nullptr;
    }

    std::stringstream json_stream;
    json_stream << "[";
    bool first_param = true;

    try {
        auto schema = scene->parameter_schema();
        PixelTheater::Log::info("get_scene_parameters_json: Found %zu parameters for scene '%s'", 
                              schema.parameters.size(), scene->name().c_str());

        for (const auto& param : schema.parameters) {
            if (!first_param) {
                json_stream << ",";
            }
            first_param = false;

            json_stream << "{";
            json_stream << "\"id\":\"" << escape_json_helper(param.name) << "\",";
            json_stream << "\"label\":\"" << escape_json_helper(param.name) << "\","; // Using name as label for now
            json_stream << "\"type\":\"" << escape_json_helper(param.type) << "\",";

            std::string controlType = "slider";
            std::string valueStr = "";
            std::vector<std::string> options;
            float min_val = 0.0f, max_val = 1.0f, step_val = 0.01f;

            if (param.type == "switch") {
                controlType = "checkbox";
                bool value = scene->settings[param.name];
                valueStr = value ? "true" : "false";
            } else if (param.type == "select") {
                controlType = "select";
                valueStr = "TODO"; // Placeholder - Need to access current selection
                options = param.options;
            } else { // Numeric types
                controlType = "slider";
                if (param.type == "count") {
                    int intValue = scene->settings[param.name];
                     if (intValue == 0 && param.default_int != 0) { intValue = param.default_int; }
                    valueStr = std::to_string(intValue);
                } else { 
                    float floatValue = scene->settings[param.name];
                    if (floatValue == 0.0f && param.default_float != 0.0f) { floatValue = param.default_float; }
                    std::stringstream ss_val;
                    ss_val.precision(6);
                    ss_val << std::fixed << floatValue;
                    valueStr = ss_val.str();
                }
                min_val = param.min_value;
                max_val = param.max_value;
                // Calculate step
                if (param.type == "ratio" || param.type == "signed_ratio") step_val = 0.01f;
                else if (param.type == "angle" || param.type == "signed_angle") step_val = M_PI / 100.0f;
                else if (param.type == "range") step_val = (max_val != min_val) ? (max_val - min_val) / 100.0f : 0.01f;
                else if (param.type == "count") step_val = 1.0f;
                else step_val = 0.01f; 
            }

            json_stream << "\"controlType\":\"" << escape_json_helper(controlType) << "\",";
            // Value needs quotes only if it's not a number or boolean
            if (controlType == "checkbox") {
                 json_stream << "\"value\":" << valueStr << ","; // Booleans don't need quotes in JSON
            } else {
                 json_stream << "\"value\":\"" << escape_json_helper(valueStr) << "\",";
            }
           
            if (controlType == "slider") {
                json_stream << "\"min\":" << min_val << ",";
                json_stream << "\"max\":" << max_val << ",";
                json_stream << "\"step\":" << step_val << ",";
            }
            if (controlType == "select") {
                json_stream << "\"options\":[";
                bool first_option = true;
                for(const auto& opt : options) {
                    if (!first_option) json_stream << ",";
                    first_option = false;
                    json_stream << "\"" << escape_json_helper(opt) << "\"";
                }
                 json_stream << "],";
            }

            // Remove trailing comma if any fields were added
             json_stream.seekp(-1, std::ios_base::end);
             json_stream << "}"; // Close param object
        }

    } catch (const std::exception& e) {
        PixelTheater::Log::error("Error creating JSON for scene parameters: %s", e.what());
        json_stream.str(""); // Clear stream on error
        json_stream << "[]"; // Return empty array on error
    }

    json_stream << "]"; // Close main array

    std::string json_string = json_stream.str();
    PixelTheater::Log::info("Generated JSON: %s", json_string.c_str());

    // Allocate memory and copy the string
    char* json_c_str = (char*)malloc(json_string.length() + 1);
    if (!json_c_str) {
        PixelTheater::Log::error("Failed to allocate memory for JSON string!");
        return nullptr;
    }
    strcpy(json_c_str, json_string.c_str());

    return json_c_str;
}

EMSCRIPTEN_KEEPALIVE
void update_scene_parameter_string(const char* param_id_cstr, const char* value_cstr) {
    // --- ADDED LOG --- 
    PixelTheater::Log::info("--> C function update_scene_parameter_string ENTERED for %s <--", param_id_cstr ? param_id_cstr : "NULL");
    // --- END ADDED LOG --- 
    
    if (!g_simulator) {
         PixelTheater::Log::error("update_scene_parameter_string: Simulator not initialized!");
         return;
    }
    if (!param_id_cstr || !value_cstr) {
        PixelTheater::Log::error("update_scene_parameter_string: Received null parameter ID or value.");
        return;
    }
    std::string param_id(param_id_cstr);
    std::string value(value_cstr);
    PixelTheater::Log::info("update_scene_parameter_string: Updating '%s' to '%s'", param_id.c_str(), value.c_str());
    g_simulator->updateSceneParameter(param_id, value); // Call the existing class method
}

} // extern "C" <-- Ensure this closes the ENTIRE block

#endif // defined(PLATFORM_WEB) || defined(EMSCRIPTEN) 