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
#include "PixelTheater/theater.h"
#include "PixelTheater/platform/web_platform.h"
#include "PixelTheater/model/model.h"
#include "PixelTheater/core/log.h" // For logging
#include "PixelTheater/params/param_value.h" // For ParamValue
#include "PixelTheater/params/param_schema.h" // For SceneParameterSchema
#include "PixelTheater/scene.h" // Correct path for Scene base class pointer
#include "benchmark.h"
#include "scenes/test_scene.h"
#include "scenes/blob_scene.h"
#include "scenes/wandering_particles/wandering_particles_scene.h"
#include "scenes/xyz_scanner/xyz_scanner_scene.h"
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

// Create a WebSimulator class to encapsulate all the functionality
class WebSimulator {
private:
    // Member variables
    std::unique_ptr<PixelTheater::Theater> theater;
    int current_scene = 0;
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
                theater->useWebPlatform<ModelDef>(); 
                
                // Apply initial settings (Platform access through Theater)
                if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                    auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
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
            
            if (theater->sceneCount() == 0) { // CORRECT: Use sceneCount()
                PixelTheater::Log::error("No scenes were added to the theater!");
                return false;
            }
            PixelTheater::Log::info("%d scenes added.", theater->sceneCount()); // Use info
            
            // Set the default scene (first scene) using Theater::setScene
            bool success = theater->setScene(0); // Set initial scene via Theater
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
            theater->update(); // THEATER handles scene tick and platform show
            BENCHMARK_END();
            
        } catch (const std::exception& e) {
            PixelTheater::Log::error("Error in main loop: %s", e.what());
        } catch (...) {
            PixelTheater::Log::error("Unknown error in main loop");
        }
    }
    
    // Scene management
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
        bool success = theater->setScene(index);

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
    
    // Set brightness
    void setBrightness(float brightness) {
        if (theater) {
            if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* webPlatform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
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
    
    // Get current brightness
    float getBrightness() {
        if (theater && theater->platform()) { // CORRECT: Use platform()
            auto* platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(theater->platform()); // CORRECT: Use platform()
             if (platform) {
                return platform->getBrightness();
             } else {
                 PixelTheater::Log::warning("WebSimulator::getBrightness: Failed to cast Platform to WebPlatform.");
             }
        }
        return 0.0f; // Default or error value
    }
    
    // Rotation management
    void updateRotation(float delta_x, float delta_y) {
        if (theater) {
            if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->updateRotation(-delta_x, -delta_y); // Inverted the deltas
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
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
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
    
    // Auto-rotation
    void setAutoRotation(bool enabled, float speed) {
        if (theater) {
            if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->setAutoRotation(enabled, speed);
                }
            }
        }
    }
    
    // Zoom levels
    void setZoomLevel(int zoom_level) {
        if (theater) {
            if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->setZoomLevel(zoom_level);
                }
            }
        }
    }
    
    // Benchmarking
    void showBenchmarkReport() {
        // (Implementation seems okay, uses emscripten_get_now and BENCHMARK_REPORT)
        static int last_frame_count = 0;
        static double last_time = emscripten_get_now() / 1000.0;
        static double fps = 60.0; // Default assumption
        
        double current_time = emscripten_get_now() / 1000.0;
        double elapsed = current_time - last_time;
        
        if (elapsed > 0.5) { // Update FPS calculation every half second
            int frame_diff = frame_count - last_frame_count;
            fps = frame_diff / elapsed;
            
            // Print benchmark report
            PixelTheater::Log::info("FPS: %.2f", fps);
            BENCHMARK_REPORT();
            
            // Reset counters
            last_frame_count = frame_count;
            last_time = current_time;
        }
    }
    
    // Debug mode toggle
    void toggleDebugMode() {
        g_debug_mode = !g_debug_mode;
        PixelTheater::Log::info("Debug mode: %s", (g_debug_mode ? "ON" : "OFF"));
    }
    
    // Get scene name by index
    void getSceneName(int scene_index, char* buffer, int buffer_size) {
        if (!buffer || buffer_size <= 0) {
            PixelTheater::Log::error("Invalid buffer provided to getSceneName");
            return;
        }
        
        std::string name = "Unknown Scene";
        
        // Use the simulator's own methods to get count and scene pointer
        int count = this->getSceneCount(); // Use simulator's getter
        if (theater && scene_index >= 0 && scene_index < count) { // Use count variable
            PixelTheater::Scene* scene = this->getScene(scene_index); // Use simulator's getter
            if (scene) {
                name = scene->name();
                PixelTheater::Log::info("Retrieved name for scene %d: '%s'", scene_index, name.c_str());
            } else {
                PixelTheater::Log::warning("Scene pointer is null for index %d", scene_index);
            }
        } else {
             PixelTheater::Log::error("Invalid scene index or theater not initialized: %d", scene_index);
        }
        
        // Copy name safely
        strncpy(buffer, name.c_str(), buffer_size - 1);
        buffer[buffer_size - 1] = '\0'; // Ensure null termination
        
        PixelTheater::Log::info("Copied name to buffer: '%s'", buffer);
    }
    
    // LED appearance settings
    void setLEDSize(float size) {
        if (theater) {
            PixelTheater::Log::info("Setting LED size to: %f", size);
            if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->setLEDSize(size);
                }
            }
        }
    }
    
    float getLEDSize() const {
        if (theater && theater->platform()) { // CORRECT: Use platform()
            auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(theater->platform()); // CORRECT: Use platform()
            if (web_platform) {
                return web_platform->getLEDSize();
            }
        }
        return 0.0f;
    }
    
    // Atmosphere effect control
    void setAtmosphereIntensity(float intensity) {
        if (theater) {
             if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
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
             auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(theater->platform()); // CORRECT: Use platform()
             if (web_platform) {
                return web_platform->getAtmosphereIntensity();
            }
        }
        return PixelTheater::WebGL::WebPlatform::DEFAULT_ATMOSPHERE_INTENSITY;
    }
    
    // Mesh visualization controls
    void setShowMesh(bool show) {
        if (theater) {
            if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->setShowMesh(show);
                     PixelTheater::Log::info("Set mesh visibility: %s", (show ? "ON" : "OFF"));
                }
            }
        }
    }
    
    bool getShowMesh() const {
        if (theater && theater->platform()) { // CORRECT: Use platform()
            auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(theater->platform()); // CORRECT: Use platform()
            if (web_platform) {
                return web_platform->getShowMesh();
            }
        }
        return false;
    }
    
    void setMeshOpacity(float opacity) {
        if (theater) {
             if (auto* platform = theater->platform()) { // CORRECT: Use platform()
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->setMeshOpacity(opacity);
                     PixelTheater::Log::info("Set mesh opacity: %.2f", opacity);
                }
            }
        }
    }
    
    float getMeshOpacity() const {
        if (theater && theater->platform()) { // CORRECT: Use platform()
             auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(theater->platform()); // CORRECT: Use platform()
             if (web_platform) {
                return web_platform->getMeshOpacity();
            }
        }
        return 0.3f; // Default value
    }
    
    // LED count
    int getLEDCount() const {
        if (theater && theater->platform()) { // CORRECT: Use platform()
            auto* platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(theater->platform());
            if(platform) return platform->getNumLEDs();
        }
        return 0;
    }
    
    // Get FPS
    float getFPS() const {
        // We could implement a better FPS counter here
        return 60.0f; // Default
    }
    
    // Get all parameters for the current scene
    std::vector<SceneParameter> getSceneParameters() {
        std::vector<SceneParameter> result;
        if (!theater) return result;
        
        auto* scene = theater->currentScene();
        if (!scene) {
             PixelTheater::Log::warning("getSceneParameters: No current scene in Theater.");
             return result;
        }
        
        try {
            auto schema = scene->parameter_schema();
            PixelTheater::Log::info("Getting parameters for scene: %s", scene->name().c_str());
            
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
                
                // Get current value via scene->settings proxy
                // Note: The original code used scene->settings().get_value() - 
                // If the proxy [] operator is intended, that should be used.
                // Assuming scene->settings is the SettingsProxy object.
                // We need to know if SettingsProxy provides .get_value() or relies on [] operator.
                // Let's assume SettingsProxy uses operator[] for now, matching TestScene fix.

                if (param.type == "switch") {
                    p.controlType = "checkbox";
                    bool value = scene->settings[param.name]; // Use operator[]
                    p.value = value ? "true" : "false";
                    PixelTheater::Log::info("  Param %s (switch): %s", param.name.c_str(), p.value.c_str());
                } 
                else if (param.type == "select") {
                    p.controlType = "select";
                    // Select might store as string or int index, assume string for now
                    // ParamValue value = scene->settings.get_value(param.name); // Assuming get_value exists on proxy for non-implicit types
                    // p.value = value.as_string(); 
                    // Let's defer fixing select/string params until we confirm proxy access method
                    p.value = "TODO: Fix select access"; // Placeholder
                    p.options = param.options;
                    PixelTheater::Log::info("  Param %s (select): %s", param.name.c_str(), p.value.c_str());
                } 
                else { // Numeric types
                    p.controlType = "slider";
                    if (param.type == "count") {
                        int intValue = scene->settings[param.name]; // Use operator[]
                        if (intValue == 0 && param.default_int != 0) { 
                             intValue = param.default_int;
                             PixelTheater::Log::info("  Param %s (count): using default %d", param.name.c_str(), intValue);
                        }
                        p.value = std::to_string(intValue);
                        PixelTheater::Log::info("  Param %s (count): %s", param.name.c_str(), p.value.c_str());
                    } else { // Other numeric (float-based)
                        float floatValue = scene->settings[param.name]; // Use operator[]
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
    
    // Update a parameter in the current scene
    void updateSceneParameter(std::string param_id, std::string value) {
        if (!theater) return;
        auto* scene = theater->currentScene();
        if (!scene) {
            PixelTheater::Log::warning("updateSceneParameter: No current scene in Theater.");
            return;
        }
        
        // Use scene's settings proxy
        auto& settings = scene->settings;
        
        if (!settings.has_parameter(param_id)) { // Assuming has_parameter is on proxy
            PixelTheater::Log::warning("Parameter not found in scene settings: %s", param_id.c_str());
            return;
        }
        
        try {
            // Get schema to know the type
            auto schema = scene->parameter_schema();
            auto it = std::find_if(schema.parameters.begin(), schema.parameters.end(),
                [&param_id](const auto& param) { return param.name == param_id; });
            
            if (it == schema.parameters.end()) {
                PixelTheater::Log::warning("Parameter not found in schema: %s", param_id.c_str());
                return;
            }
            
            // Update using the proxy's operator[] = overload
            if (it->type == "switch") {
                settings[param_id] = (value == "true");
            } else if (it->type == "select") {
                // Select might need explicit ParamValue construction or string assignment?
                 PixelTheater::Log::warning("TODO: Fix select parameter update for %s", param_id.c_str());
                 // settings[param_id] = value; // ??? Need to confirm proxy assignment
            } else if (it->type == "count") {
                 settings[param_id] = std::stoi(value); 
            } else { // Float types
                 settings[param_id] = std::stof(value);
            }
            
            PixelTheater::Log::info("Updated parameter %s to %s", param_id.c_str(), value.c_str());

        } catch (const std::invalid_argument& ia) {
            PixelTheater::Log::error("Invalid argument for stoi/stof on param %s: %s", param_id.c_str(), ia.what());
        } catch (const std::out_of_range& oor) {
            PixelTheater::Log::error("Out of range for stoi/stof on param %s: %s", param_id.c_str(), oor.what());
        } catch (const std::exception& e) {
            PixelTheater::Log::error("Error updating parameter %s: %s", param_id.c_str(), e.what());
        }
    }

    // Add this public method to the WebSimulator class (in the public: section)
    // This method also needs updating if Scene base class changed
    PixelTheater::Scene* getScene(int scene_index) { // Return base Scene*
        if (!theater) return nullptr;
        if (scene_index < 0 || static_cast<size_t>(scene_index) >= theater->sceneCount()) { // CORRECT: Use sceneCount()
            return nullptr;
        }
        // Theater::scene returns Scene&, return its address
        return &theater->scene(static_cast<size_t>(scene_index)); 
    }
};

// Create a global WebSimulator instance
static std::unique_ptr<WebSimulator> g_simulator;

// Forward declarations of functions used in main
extern "C" {
    bool init_simulator();
    void update_simulator();
}

// Main function (not extern "C")
int main() {
    // Initialize the simulator
    if (!init_simulator()) {
        std::cerr << "Failed to initialize simulator" << std::endl;
        return 1;
    }
    
    // Set up the animation loop
    emscripten_set_main_loop([]() {
        update_simulator();
    }, 0, 1);  // 0 = no FPS limit, 1 = simulate infinite loop
    
    return 0;
}

// JavaScript interface functions
extern "C" {

EMSCRIPTEN_KEEPALIVE
bool init_simulator() {
    PixelTheater::Log::info("init_simulator() called.");
    if (g_simulator) {
        PixelTheater::Log::warning("Simulator already initialized.");
        return true; // Or false? Depending on desired re-init behavior
    }
    try {
        g_simulator = std::make_unique<WebSimulator>();
        bool success = g_simulator->initialize();
        if (success) {
            PixelTheater::Log::info("Simulator initialized successfully.");
        } else {
             PixelTheater::Log::error("Simulator initialization failed.");
        }
        return success;
    } catch (const std::exception& e) {
        PixelTheater::Log::error("Exception during simulator initialization: %s", e.what());
        g_simulator.reset(); // Ensure simulator is null on failure
        return false;
    } catch (...) {
        PixelTheater::Log::error("Unknown exception during simulator initialization.");
        g_simulator.reset();
        return false;
    }
}

EMSCRIPTEN_KEEPALIVE
void update_simulator() {
    if (g_simulator) {
        g_simulator->update();
    }
}

EMSCRIPTEN_KEEPALIVE
void change_scene(int sceneIndex) {
    if (g_simulator) {
        g_simulator->setScene(sceneIndex);
    } else {
        PixelTheater::Log::error("change_scene called before simulator initialized.");
    }
}

EMSCRIPTEN_KEEPALIVE
int get_num_scenes() {
    return g_simulator ? g_simulator->getSceneCount() : 0;
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
        // Toggle global flag anyway? Or require init?
        // g_debug_mode = !g_debug_mode; 
    }
}

EMSCRIPTEN_KEEPALIVE
const char* get_scene_name(int scene_index) {
    static std::string sceneNameStr = "Invalid Scene"; // Default/error value
    if (g_simulator) {
        // Get the scene pointer using the simulator's helper method
        PixelTheater::Scene* scene = g_simulator->getScene(scene_index); // CORRECT: Use simulator's getScene
        if (scene) {
             sceneNameStr = scene->name(); // Get name from the Scene object
             return sceneNameStr.c_str();
        } else {
            PixelTheater::Log::warning("get_scene_name: getScene(%d) returned null.", scene_index);
        }
    } else {
         PixelTheater::Log::error("get_scene_name called before simulator initialized.");
    }
    // Return default/error value if simulator not ready or scene not found
    return sceneNameStr.c_str(); 
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
    return g_simulator ? g_simulator->getAtmosphereIntensity() : PixelTheater::WebGL::WebPlatform::DEFAULT_ATMOSPHERE_INTENSITY;
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
int get_led_count() {
    return g_simulator ? g_simulator->getLEDCount() : 0;
}

EMSCRIPTEN_KEEPALIVE
float get_fps() {
    return g_simulator ? g_simulator->getFPS() : 0.0f;
}

EMSCRIPTEN_KEEPALIVE
void log_message(const char* message) {
    // Simple passthrough for JS logging
    PixelTheater::Log::info("[JS] %s", message);
}

// NEW C function to handle canvas resize events from JS
EMSCRIPTEN_KEEPALIVE
void resize_canvas(int width, int height) {
    if (g_simulator) {
        // Assuming WebSimulator has an onCanvasResize method
        // Need to verify WebSimulator has this method and its signature
        // g_simulator->onCanvasResize(width, height); 
        PixelTheater::Log::info("resize_canvas called: %d x %d (WebSimulator::onCanvasResize not yet implemented/called)", width, height);
    } else {
        PixelTheater::Log::error("resize_canvas called before simulator initialized.");
    }
}

} // extern "C"

// Helper functions for Emscripten bindings
emscripten::val get_scene_parameters_wrapper() {
    if (!g_simulator) return emscripten::val::array();
    
    auto params = g_simulator->getSceneParameters(); // Calls the C++ method
    auto result = emscripten::val::array();
    
    for (size_t i = 0; i < params.size(); i++) {
        auto param = emscripten::val::object();
        param.set("id", params[i].id);
        param.set("label", params[i].label);
        param.set("controlType", params[i].controlType);
        param.set("value", params[i].value); // Value is already stringified
        param.set("type", params[i].type);  
        param.set("min", params[i].min);
        param.set("max", params[i].max);
        param.set("step", params[i].step);
        
        // Convert options vector to JS array if present
        if (!params[i].options.empty()) {
             auto options = emscripten::val::array();
             for (size_t j = 0; j < params[i].options.size(); j++) {
                 options.set(j, params[i].options[j]);
             }
             param.set("options", options);
        } else {
            param.set("options", emscripten::val::null()); // Use null if no options
        }
        
        result.set(i, param);
    }
    
    return result;
}

void update_scene_parameter_wrapper(std::string param_id, std::string value) {
    if (!g_simulator) return;
    g_simulator->updateSceneParameter(param_id, value); // Calls the C++ method
}

EMSCRIPTEN_BINDINGS(scene_parameters) {
    using namespace emscripten;
    
    // Bind the wrapper functions for parameter handling
    function("getSceneParameters", &get_scene_parameters_wrapper);
    function("updateSceneParameter", &update_scene_parameter_wrapper);
}

// NOTE: Other simple C functions are exported via EXPORTED_FUNCTIONS in the Makefile/
// build script, not necessarily requiring explicit EMSCRIPTEN_BINDINGS here unless
// complex types (like std::string return for get_scene_name) need special handling.

// Binding get_scene_name (returns const char*) doesn't strictly need EMSCRIPTEN_BINDINGS
// if it's listed in EXPORTED_FUNCTIONS, but doesn't hurt.
// EMSCRIPTEN_BINDINGS(scene_names) {
//     emscripten::function("get_scene_name", &get_scene_name);
// }

#endif // defined(PLATFORM_WEB) || defined(EMSCRIPTEN) 