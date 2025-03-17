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
#include "PixelTheater/stage.h"
#include "PixelTheater/platform/web_platform.h"
#include "PixelTheater/model/model.h"
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
    std::unique_ptr<PixelTheater::WebGL::WebPlatform> platform;
    std::unique_ptr<PixelTheater::Model<ModelDef>> model;
    std::unique_ptr<PixelTheater::Stage<ModelDef>> stage;
    int current_scene = 0;
    int frame_count = 0;
    
public:
    // Constructor
    WebSimulator() {
        std::cout << "Creating WebSimulator instance..." << std::endl;
    }
    
    // Initialize the simulator
    bool initialize() {
        try {
            // Enable benchmarking
            Benchmark::enabled = true;
            
            // Initialize the platform
            if (!platform) {
                platform = std::make_unique<PixelTheater::WebGL::WebPlatform>();
                platform->initializeWithModel<ModelDef>();
                platform->setBrightness(200);
                platform->setZoomLevel(1); 
                
                printf("Platform initialized successfully\n");
            }
            
            // Create model instance
            if (!model) {
                model = std::make_unique<PixelTheater::Model<ModelDef>>(ModelDef{}, platform->getLEDs());
                printf("Model created successfully\n");
            }
            
            // Create stage with platform and model
            if (!stage) {
                stage = std::make_unique<PixelTheater::Stage<ModelDef>>(std::move(platform), std::move(model));
                printf("Stage created successfully\n");
            }
            
            // Add scenes
            if (g_debug_mode) {
                std::cout << "Adding scenes..." << std::endl;
            }
            
            // Add all scenes to the stage
            stage->template addScene<Scenes::TestScene<ModelDef>>(*stage);
            stage->template addScene<Scenes::BlobScene<ModelDef>>(*stage);
            stage->template addScene<Scenes::WanderingParticlesScene<ModelDef>>(*stage);
            stage->template addScene<Scenes::XYZScannerScene<ModelDef>>(*stage);
            
            // Set up all scenes
            for (size_t i = 0; i < stage->getSceneCount(); i++) {
                PixelTheater::Scene<ModelDef>* scene = stage->getScene(i);
                if (scene) {
                    scene->setup();
                }
            }
            
            // Set the default scene (first scene)
            if (stage->getSceneCount() > 0) {
                PixelTheater::Scene<ModelDef>* firstScene = stage->getScene(0);
                if (firstScene) {
                    stage->setScene(firstScene);
                    current_scene = 0;
                    
                    if (g_debug_mode) {
                        std::cout << "Initial scene: " << firstScene->name() << std::endl;
                    }
                }
            }
            
            // Reset benchmark data
            BENCHMARK_RESET();
            
            return true;
        } catch (const std::exception& e) {
            printf("Error during initialization: %s\n", e.what());
            return false;
        }
    }
    
    // Main update function called every frame
    void update() {
        if (!stage) return;
        
        try {
            frame_count++;
            
            // Print debug info every 600 frames (10 seconds at 60fps) when debug mode is on
            if (frame_count % 600 == 0 && g_debug_mode) {
                std::stringstream ss;
                ss << "Frame " << frame_count;
                
                // Add null checks and bounds checking
                if (stage && stage->model.face_count() > 0) {
                    ss << ", Model faces: " << stage->model.face_count();
                }
                
                if (stage->getPlatform()) {
                    ss << ", LED count: " << stage->getPlatform()->getNumLEDs();
                }
                
                std::cout << ss.str() << std::endl;
            }
            
            // Update and render the current scene with careful error handling
            if (stage) {
                BENCHMARK_START("update");
                stage->update();
                BENCHMARK_END();
            }
            
            if (auto* platform = stage->getPlatform()) {
                BENCHMARK_START("show");
                platform->show();
                BENCHMARK_END();
            }
        } catch (const std::exception& e) {
            std::cerr << "Error in main loop: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown error in main loop" << std::endl;
        }
    }
    
    // Scene management
    void setScene(int sceneIndex) {
        if (!stage) {
            std::cerr << "Stage not initialized" << std::endl;
            return;
        }
        
        std::cout << "Scene change requested to index: " << sceneIndex << std::endl;
        
        // Store the current scene index to check if we're actually changing scenes
        int previous_scene = current_scene;
        
        // Validate scene index
        if (sceneIndex < 0 || sceneIndex >= static_cast<int>(stage->getSceneCount())) {
            std::cerr << "Invalid scene index: " << sceneIndex << std::endl;
            return;
        }
        
        // Get the target scene pointer
        PixelTheater::Scene<ModelDef>* targetScene = stage->getScene(sceneIndex);
        
        // Only proceed if we have a valid target scene
        if (targetScene) {
            // Set the scene
            stage->setScene(targetScene);
            current_scene = sceneIndex;
            std::cout << "Changed to scene: " << targetScene->name() << std::endl;
            
            // Update the current scene index
            if (previous_scene != current_scene) {
                // Camera reset functionality is not available in WebPlatform
                // Just update the scene index
            }
        } else {
            std::cerr << "Scene at index " << sceneIndex << " is null" << std::endl;
        }
    }
    
    // Get the number of available scenes
    int getSceneCount() const {
        return stage ? static_cast<int>(stage->getSceneCount()) : 0;
    }
    
    // Set brightness
    void setBrightness(uint8_t brightness) {
        if (stage) {
            std::cout << "Setting brightness to: " << (int)brightness << std::endl;
            auto* platform = stage->getPlatform();
            if (platform) {
                platform->setBrightness(brightness);
            } else {
                std::cerr << "Platform not initialized for brightness setting" << std::endl;
            }
        } else {
            std::cerr << "Stage not initialized for brightness setting" << std::endl;
        }
    }
    
    // Get current brightness
    uint8_t getBrightness() {
        if (stage && stage->getPlatform()) {
            auto* platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(stage->getPlatform());
            if (platform) {
                return platform->getBrightness();
            }
        }
        return PixelTheater::WebGL::WebPlatform::DEFAULT_BRIGHTNESS;
    }
    
    // Rotation management
    void updateRotation(float delta_x, float delta_y) {
        if (stage) {
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->updateRotation(-delta_x, -delta_y); // Inverted the deltas
                } else {
                    std::cerr << "Platform is not a WebPlatform" << std::endl;
                }
            } else {
                std::cerr << "Platform not initialized for rotation update" << std::endl;
            }
        } else {
            std::cerr << "Stage not initialized for rotation update" << std::endl;
        }
    }
    
    void resetRotation() {
        if (stage) {
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->resetRotation();
                } else {
                    std::cerr << "Platform is not a WebPlatform" << std::endl;
                }
            } else {
                std::cerr << "Platform not initialized for rotation reset" << std::endl;
            }
        } else {
            std::cerr << "Stage not initialized for rotation reset" << std::endl;
        }
    }
    
    // Auto-rotation
    void setAutoRotation(bool enabled, float speed) {
        if (stage) {
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->setAutoRotation(enabled, speed);
                }
            }
        }
    }
    
    // Zoom levels
    void setZoomLevel(int zoom_level) {
        if (stage) {
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->setZoomLevel(zoom_level);
                }
            }
        }
    }
    
    // Benchmarking
    void showBenchmarkReport() {
        // Calculate approximate FPS based on frame count
        static int last_frame_count = 0;
        static double last_time = emscripten_get_now() / 1000.0;
        static double fps = 60.0; // Default assumption
        
        double current_time = emscripten_get_now() / 1000.0;
        double elapsed = current_time - last_time;
        
        if (elapsed > 0.5) { // Update FPS calculation every half second
            int frame_diff = frame_count - last_frame_count;
            fps = frame_diff / elapsed;
            
            // Print benchmark report
            std::cout << "FPS: " << fps << std::endl;
            BENCHMARK_REPORT();
            
            // Reset counters
            last_frame_count = frame_count;
            last_time = current_time;
        }
    }
    
    // Debug mode toggle
    void toggleDebugMode() {
        g_debug_mode = !g_debug_mode;
        std::cout << "Debug mode: " << (g_debug_mode ? "ON" : "OFF") << std::endl;
    }
    
    // Model info
    void printModelInfo() {
        if (stage) {
            std::cout << "Model Information:" << std::endl;
            std::cout << "  Face count: " << stage->model.face_count() << std::endl;
            
            // Print information about each face
            for (size_t i = 0; i < stage->model.face_count(); i++) {
                auto& face = stage->model.faces[i];
                std::cout << "  Face " << i << ": " << face.led_count() << " LEDs" << std::endl;
            }
        } else {
            std::cout << "Model not initialized" << std::endl;
        }
    }
    
    // Get scene name by index
    void getSceneName(int scene_index, char* buffer, int buffer_size) {
        // Check for null buffer or invalid buffer size
        if (!buffer || buffer_size <= 0) {
            std::cerr << "Invalid buffer provided to getSceneName" << std::endl;
            return;
        }
        
        std::string name = "Unknown Scene";
        
        // Get the name from the scene if possible
        if (stage && scene_index >= 0 && scene_index < static_cast<int>(stage->getSceneCount())) {
            auto* scene = stage->getScene(scene_index);
            if (scene) {
                name = scene->name();
                std::cout << "Retrieved name for scene " << scene_index << ": '" << name << "'" << std::endl;
            } else {
                std::cerr << "Scene pointer is null for index " << scene_index << std::endl;
            }
        } else {
            std::cerr << "Invalid scene index or stage not initialized: " << scene_index << std::endl;
        }
        
        // Copy the name to the provided buffer safely
        int name_length = name.length();
        int copy_length = (name_length < buffer_size - 1) ? name_length : buffer_size - 1;
        
        for (int i = 0; i < copy_length; i++) {
            buffer[i] = name[i];
        }
        buffer[copy_length] = '\0'; // Ensure null termination
        
        std::cout << "Copied name to buffer: '" << buffer << "'" << std::endl;
    }
    
    // LED appearance settings
    void setLEDSize(float size) {
        if (stage) {
            std::cout << "Setting LED size to: " << size << std::endl;
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->setLEDSize(size);
                }
            }
        }
    }
    
    float getLEDSize() const {
        if (stage) {
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
                if (web_platform) {
                    return web_platform->getLEDSize();
                }
            }
        }
        return 0.0f;
    }
    
    // Atmosphere effect control
    void setAtmosphereIntensity(float intensity) {
        if (stage) {
            std::cout << "Setting atmosphere intensity to: " << intensity << std::endl;
            auto* platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(stage->getPlatform());
            if (platform) {
                platform->setAtmosphereIntensity(intensity);
            } else {
                std::cerr << "Platform not initialized for atmosphere setting" << std::endl;
            }
        }
    }
    
    float getAtmosphereIntensity() const {
        if (stage) {
            auto* platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(stage->getPlatform());
            if (platform) {
                return platform->getAtmosphereIntensity();
            }
        }
        return PixelTheater::WebGL::WebPlatform::DEFAULT_ATMOSPHERE_INTENSITY;
    }
    
    // Mesh visualization controls
    void setShowMesh(bool show) {
        if (stage) {
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->setShowMesh(show);
                    std::cout << "Set mesh visibility: " << (show ? "ON" : "OFF") << std::endl;
                }
            }
        }
    }
    
    bool getShowMesh() const {
        if (stage) {
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
                if (web_platform) {
                    return web_platform->getShowMesh();
                }
            }
        }
        return false;
    }
    
    void setMeshOpacity(float opacity) {
        if (stage) {
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->setMeshOpacity(opacity);
                    std::cout << "Set mesh opacity: " << opacity << std::endl;
                }
            }
        }
    }
    
    float getMeshOpacity() const {
        if (stage) {
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
                if (web_platform) {
                    return web_platform->getMeshOpacity();
                }
            }
        }
        return 0.3f; // Default value
    }
    
    // LED count
    int getLEDCount() const {
        if (stage && stage->getPlatform()) {
            return stage->getPlatform()->getNumLEDs();
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
        
        // Get the current scene
        auto* scene = stage->getCurrentScene();
        if (!scene) return result;
        
        try {
            // Get the parameter schema from the scene
            auto schema = scene->parameter_schema();
            
            PixelTheater::Log::warning("Getting parameters for scene: %s", scene->name().c_str());
            
            // Process each parameter in the schema
            for (const auto& param : schema.parameters) {
                // Create a SceneParameter object
                SceneParameter p;
                p.id = param.name;
                p.label = param.name;
                p.type = param.type;  // Add the parameter type
                
                // Calculate step size based on parameter type and range
                auto calculateStepSize = [](const auto& param) -> float {
                    // For ratio types (0-1 range), use 0.01 (100 steps)
                    if (param.type == "ratio" || param.type == "signed_ratio") {
                        return 0.01f;
                    }
                    
                    // For angle types, use PI/100 (100 steps)
                    if (param.type == "angle" || param.type == "signed_angle") {
                        return M_PI / 100.0f;
                    }
                    
                    // For custom ranges, use (max-min)/100 for float values
                    if (param.type == "range") {
                        return (param.max_value - param.min_value) / 100.0f;
                    }
                    
                    // For count type (integers), use 1
                    if (param.type == "count") {
                        return 1.0f;
                    }
                    
                    // Default to 0.01 for unknown types
                    return 0.01f;
                };
                
                // Set control type and properties based on parameter type
                if (param.type == "switch") {
                    p.controlType = "checkbox";
                    // Get the current value
                    const auto& value = scene->_settings_storage.get_value(param.name);
                    p.value = value.as_bool() ? "true" : "false";
                    PixelTheater::Log::warning("  Parameter %s (switch): %s", param.name.c_str(), p.value.c_str());
                } 
                else if (param.type == "select") {
                    p.controlType = "select";
                    const auto& value = scene->_settings_storage.get_value(param.name);
                    p.value = value.as_string();
                    p.options = param.options;
                    PixelTheater::Log::warning("  Parameter %s (select): %s", param.name.c_str(), p.value.c_str());
                } 
                else {
                    // All numeric types use a slider
                    p.controlType = "slider";
                    const auto& value = scene->_settings_storage.get_value(param.name);
                    
                    // Special handling for count type parameters
                    if (param.type == "count") {
                        // Format as integer
                        int intValue = static_cast<int>(value.as_float());
                        
                        // If the value is zero, use the default value from the schema
                        if (intValue == 0) {
                            intValue = param.default_int;
                            PixelTheater::Log::warning("  Parameter %s (count): using default %d instead of 0", 
                                param.name.c_str(), intValue);
                        }
                        
                        p.value = std::to_string(intValue);
                        PixelTheater::Log::warning("  Parameter %s (count): %s (from %f)", 
                            param.name.c_str(), p.value.c_str(), value.as_float());
                    } else {
                        // Format as float
                        float floatValue = value.as_float();
                        
                        // If the value is zero and this is not the speed parameter, use the default value
                        if (floatValue == 0.0f && param.name != "speed") {
                            floatValue = param.default_float;
                            PixelTheater::Log::warning("  Parameter %s (%s): using default %f instead of 0", 
                                param.name.c_str(), param.type.c_str(), floatValue);
                        }
                        
                        // Format float value with full precision
                        std::stringstream ss;
                        ss.precision(6);  // Use 6 decimal places
                        ss << std::fixed << floatValue;
                        p.value = ss.str();
                        
                        PixelTheater::Log::warning("  Parameter %s (%s): %s (raw: %.6f)", 
                            param.name.c_str(), param.type.c_str(), p.value.c_str(), floatValue);
                    }
                    
                    // Set min/max values
                    p.min = param.min_value;
                    p.max = param.max_value;
                    
                    // Calculate step size based on parameter type and range
                    p.step = calculateStepSize(param);
                }
                
                result.push_back(p);
            }
        } catch (const std::exception& e) {
            PixelTheater::Log::warning("Error getting scene parameters: %s", e.what());
        }
        
        return result;
    }
    
    // Update a parameter in the current scene
    void updateSceneParameter(std::string param_id, std::string value) {
        // Get the current scene
        auto* scene = stage->getCurrentScene();
        if (!scene) return;
        
        // Get the scene's settings
        auto& settings = scene->_settings_storage;
        
        // Check if parameter exists
        if (!settings.has_parameter(param_id)) {
            PixelTheater::Log::warning("Parameter not found: %s", param_id.c_str());
            return;
        }
        
        try {
            // Get parameter schema to determine type
            auto schema = scene->parameter_schema();
            
            // Find the parameter in the schema
            auto it = std::find_if(schema.parameters.begin(), schema.parameters.end(),
                [&param_id](const auto& param) { return param.name == param_id; });
            
            if (it == schema.parameters.end()) {
                PixelTheater::Log::warning("Parameter not found in schema: %s", param_id.c_str());
                return;
            }
            
            // Convert value based on parameter type
            PixelTheater::ParamValue paramValue;
            if (it->type == "switch") {
                // Convert string "true"/"false" to bool
                paramValue = PixelTheater::ParamValue(value == "true");
            } else if (it->type == "select") {
                // For select, pass the string value
                paramValue = PixelTheater::ParamValue(value.c_str());
            } else if (it->type == "count") {
                // For count type, convert to integer
                paramValue = PixelTheater::ParamValue(static_cast<int>(std::stof(value)));
                PixelTheater::Log::warning("Setting count parameter %s to %d", param_id.c_str(), static_cast<int>(std::stof(value)));
            } else {
                // For float types (ratio, angle, range), preserve decimal precision
                float floatValue = std::stof(value);
                
                // Log the raw value for debugging
                PixelTheater::Log::warning("Raw float value for %s (%s): %.6f", 
                    param_id.c_str(), it->type.c_str(), floatValue);
                
                paramValue = PixelTheater::ParamValue(floatValue);
                PixelTheater::Log::warning("Setting float parameter %s (%s) to %.6f", 
                    param_id.c_str(), it->type.c_str(), floatValue);
            }
            
            // Update the parameter
            settings.set_value(param_id, paramValue);
            
            // Don't reset the scene, just log the update
            PixelTheater::Log::warning("Updated parameter %s to %s", param_id.c_str(), value.c_str());
            
            // Call setup to ensure the scene is updated with the new parameter value
            // This is important for parameters that affect the scene's behavior
            if (param_id == "num_blobs" || param_id == "min_radius" || param_id == "max_radius" || 
                param_id == "max_age" || param_id == "fade") {
                scene->setup();
            }
        } catch (const std::exception& e) {
            PixelTheater::Log::warning("Error updating parameter %s: %s", param_id.c_str(), e.what());
        }
    }

    // Add this public method to the WebSimulator class (in the public: section)
    PixelTheater::Scene<ModelDef>* getScene(int scene_index) {
        return stage ? stage->getScene(scene_index) : nullptr;
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
    try {
        g_simulator = std::make_unique<WebSimulator>();
        return g_simulator->initialize();
    } catch (const std::exception& e) {
        std::cerr << "Error initializing simulator: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown error initializing simulator" << std::endl;
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
    }
}

EMSCRIPTEN_KEEPALIVE
int get_scene_count() {
    return g_simulator ? g_simulator->getSceneCount() : 0;
}

EMSCRIPTEN_KEEPALIVE
void set_brightness(uint8_t brightness) {
    if (g_simulator) {
        g_simulator->setBrightness(brightness);
    }
}

EMSCRIPTEN_KEEPALIVE
uint8_t get_brightness() {
    return g_simulator ? g_simulator->getBrightness() : 0;
}

EMSCRIPTEN_KEEPALIVE
void update_rotation(float delta_x, float delta_y) {
    if (g_simulator) {
        g_simulator->updateRotation(delta_x, delta_y);
    }
}

EMSCRIPTEN_KEEPALIVE
void reset_rotation() {
    if (g_simulator) {
        g_simulator->resetRotation();
    }
}

EMSCRIPTEN_KEEPALIVE
void set_auto_rotation(bool enabled, float speed) {
    if (g_simulator) {
        g_simulator->setAutoRotation(enabled, speed);
    }
}

EMSCRIPTEN_KEEPALIVE
void set_zoom_level(int zoom_level) {
    if (g_simulator) {
        g_simulator->setZoomLevel(zoom_level);
    }
}

EMSCRIPTEN_KEEPALIVE
void show_benchmark_report() {
    if (g_simulator) {
        g_simulator->showBenchmarkReport();
    }
}

EMSCRIPTEN_KEEPALIVE
void toggle_debug_mode() {
    if (g_simulator) {
        g_simulator->toggleDebugMode();
    }
}

EMSCRIPTEN_KEEPALIVE
void print_model_info() {
    if (g_simulator) {
        g_simulator->printModelInfo();
    }
}

EMSCRIPTEN_KEEPALIVE
int get_num_scenes() {
    return g_simulator ? g_simulator->getSceneCount() : 0;
}

EMSCRIPTEN_KEEPALIVE
std::string get_scene_name(int scene_index) {
    if (!g_simulator) return "Unknown Scene";
    
    try {
        auto* scene = g_simulator->getScene(scene_index);
        if (!scene) {
            return "Unknown Scene";
        }
        return scene->name();
    } catch (const std::exception& e) {
        std::cerr << "Error getting scene name: " << e.what() << std::endl;
        return "Unknown Scene";
    }
}

EMSCRIPTEN_KEEPALIVE
void set_led_size(float size) {
    if (g_simulator) {
        g_simulator->setLEDSize(size);
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
    }
}

EMSCRIPTEN_KEEPALIVE
float get_atmosphere_intensity() {
    return g_simulator ? g_simulator->getAtmosphereIntensity() : 0.0f;
}

EMSCRIPTEN_KEEPALIVE
void set_show_mesh(bool show) {
    if (g_simulator) {
        g_simulator->setShowMesh(show);
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
    PixelTheater::Log::warning("%s", message);
}

} // extern "C"

// Helper functions for Emscripten bindings
emscripten::val get_scene_parameters_wrapper() {
    if (!g_simulator) return emscripten::val::array();
    
    auto params = g_simulator->getSceneParameters();
    auto result = emscripten::val::array();
    
    for (size_t i = 0; i < params.size(); i++) {
        auto param = emscripten::val::object();
        param.set("id", params[i].id);
        param.set("label", params[i].label);
        param.set("controlType", params[i].controlType);
        param.set("value", params[i].value);
        param.set("type", params[i].type);  // Add the type field
        param.set("min", params[i].min);
        param.set("max", params[i].max);
        param.set("step", params[i].step);
        
        // Convert options vector to JS array
        auto options = emscripten::val::array();
        for (size_t j = 0; j < params[i].options.size(); j++) {
            options.set(j, params[i].options[j]);
        }
        param.set("options", options);
        
        result.set(i, param);
    }
    
    return result;
}

void update_scene_parameter_wrapper(std::string param_id, std::string value) {
    if (!g_simulator) return;
    g_simulator->updateSceneParameter(param_id, value);
}

EMSCRIPTEN_BINDINGS(scene_parameters) {
    using namespace emscripten;
    
    // Bind the wrapper functions
    function("getSceneParameters", &get_scene_parameters_wrapper);
    function("updateSceneParameter", &update_scene_parameter_wrapper);
}

// Add this binding to properly handle string returns
EMSCRIPTEN_BINDINGS(scene_names) {
    emscripten::function("get_scene_name", &get_scene_name);
}

#endif // defined(PLATFORM_WEB) || defined(EMSCRIPTEN) 