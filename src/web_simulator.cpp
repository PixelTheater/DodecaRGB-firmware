#if defined(PLATFORM_WEB) || defined(EMSCRIPTEN)
// This file is only used in web builds

#include <emscripten.h>
#include <emscripten/html5.h>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include "PixelTheater/stage.h"
#include "PixelTheater/platform/web_platform.h"
#include "PixelTheater/model/model.h"
#include "benchmark.h"
#include "scenes/all_scenes.h"
#include "scenes/test_scene.h"
#include "scenes/blob_scene.h"

#ifndef DODECARGBV2_MODEL_INCLUDED
#define DODECARGBV2_MODEL_INCLUDED
#include "model.cpp" // Include the generated model
#endif

// Define global debug flag needed by the library
bool g_debug_mode = false;

// Define the model type we're using - use the fully qualified name
using ModelDef = PixelTheater::Fixtures::DodecaRGBv2;

// Create a WebSimulator class to encapsulate all the functionality
class WebSimulator {
private:
    // Member variables
    std::unique_ptr<PixelTheater::WebPlatform> platform;
    std::unique_ptr<PixelTheater::Model<ModelDef>> model;
    std::unique_ptr<PixelTheater::Stage<ModelDef>> stage;
    Scenes::TestScene<ModelDef>* test_scene = nullptr;
    Scenes::BlobScene<ModelDef>* blob_scene = nullptr;
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
            
            // Create the platform with the correct number of LEDs
            const uint16_t num_leds = ModelDef::LED_COUNT;
            if (g_debug_mode) {
                std::cout << "Creating WebPlatform with " << num_leds << " LEDs" << std::endl;
            }
            platform = std::make_unique<PixelTheater::WebPlatform>(num_leds);
            
            // Set higher default brightness for better visibility
            platform->setBrightness(200);
            
            // Set initial zoom level to NORMAL
            platform->setZoomLevel(static_cast<int>(PixelTheater::ZoomLevel::NORMAL));
            
            // Create model based on our model definition
            if (g_debug_mode) {
                std::cout << "Creating Model" << std::endl;
            }
            model = std::make_unique<PixelTheater::Model<ModelDef>>(ModelDef{}, platform->getLEDs());
            
            // Print model information
            if (g_debug_mode) {
                std::cout << "Model created with " << model->face_count() << " faces" << std::endl;
            }
            
            // Store a raw pointer to the model for the coordinate provider
            // This is safe because the model will be owned by the stage and will outlive the platform
            auto* model_ptr = model.get();
            
            // Set up the coordinate provider callback
            // This will provide the 3D coordinates for each LED from the model
            platform->setCoordinateProvider([model_ptr](uint16_t index, float& x, float& y, float& z) {
                if (model_ptr && index < model_ptr->led_count()) {
                    const auto& point = model_ptr->points[index];
                    x = static_cast<float>(point.x());
                    y = static_cast<float>(point.y());
                    z = static_cast<float>(point.z());
                } else {
                    // Default values if model or index is invalid
                    x = 0.0f;
                    y = 0.0f;
                    z = 0.0f;
                }
            });
            
            // Create stage with platform and model
            if (g_debug_mode) {
                std::cout << "Creating Stage" << std::endl;
            }
            stage = std::make_unique<PixelTheater::Stage<ModelDef>>(std::move(platform), std::move(model));
            
            // Add scenes
            if (g_debug_mode) {
                std::cout << "Adding scenes..." << std::endl;
            }
            test_scene = stage->template addScene<Scenes::TestScene<ModelDef>>(*stage);
            blob_scene = stage->template addScene<Scenes::BlobScene<ModelDef>>(*stage);
            
            // Set up scenes
            test_scene->setup();
            blob_scene->setup();
            
            // Set initial scene to BlobScene
            stage->setScene(blob_scene);
            current_scene = 1;
            
            if (g_debug_mode) {
                std::cout << "Initial scene: Blob Scene" << std::endl;
            }
            
            // Reset benchmark data
            BENCHMARK_RESET();
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error in initialization: " << e.what() << std::endl;
            return false;
        } catch (...) {
            std::cerr << "Unknown error in initialization" << std::endl;
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
        
        switch (sceneIndex) {
            case 0:
                if (test_scene) {
                    stage->setScene(test_scene);
                    current_scene = 0;
                    std::cout << "Changed to Test Scene" << std::endl;
                }
                break;
            case 1:
                if (blob_scene) {
                    stage->setScene(blob_scene);
                    current_scene = 1;
                    std::cout << "Changed to Blob Scene" << std::endl;
                }
                break;
            default:
                std::cerr << "Invalid scene index: " << sceneIndex << std::endl;
                break;
        }
    }
    
    // Get the number of available scenes
    int getSceneCount() const {
        return 2; // TestScene and BlobScene
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
    uint8_t getBrightness() const {
        if (stage) {
            auto* platform = dynamic_cast<PixelTheater::WebPlatform*>(stage->getPlatform());
            if (platform) {
                return platform->getBrightness();
            }
        }
        // Return default if platform not available
        return PixelTheater::WebPlatform::DEFAULT_BRIGHTNESS;
    }
    
    // Rotation management
    void updateRotation(float delta_x, float delta_y) {
        if (stage) {
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->updateRotation(delta_x, delta_y);
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
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform);
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
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->setAutoRotation(enabled, speed);
                }
            }
        }
    }
    
    // View presets
    void setPresetView(int preset_index) {
        if (stage) {
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->setPresetView(preset_index);
                }
            }
        }
    }
    
    // Zoom levels
    void setZoomLevel(int zoom_level) {
        if (stage) {
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform);
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
            
            last_frame_count = frame_count;
            last_time = current_time;
        }
        
        // Show benchmark report with calculated FPS
        BENCHMARK_REPORT(fps);
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
        
        const char* name = "Unknown";
        switch (scene_index) {
            case 0:
                name = "Test Scene";
                break;
            case 1:
                name = "Blob Scene";
                break;
        }
        
        // Copy the name to the provided buffer safely
        int name_length = strlen(name);
        int copy_length = (name_length < buffer_size - 1) ? name_length : buffer_size - 1;
        
        for (int i = 0; i < copy_length; i++) {
            buffer[i] = name[i];
        }
        buffer[copy_length] = '\0'; // Ensure null termination
        
        std::cout << "Copied scene name: " << name << " (index: " << scene_index << ")" << std::endl;
    }
    
    // LED appearance settings
    void setLEDSize(float size) {
        if (stage) {
            std::cout << "Setting LED size to: " << size << std::endl;
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform);
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
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform);
                if (web_platform) {
                    return web_platform->getLEDSize();
                }
            }
        }
        return 0.0f;
    }
    
    void setBloomIntensity(float intensity) {
        if (stage) {
            std::cout << "Setting bloom intensity to: " << intensity << std::endl;
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform);
                if (web_platform) {
                    web_platform->setBloomIntensity(intensity);
                }
            }
        }
    }
    
    float getBloomIntensity() const {
        if (stage) {
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebPlatform*>(platform);
                if (web_platform) {
                    return web_platform->getBloomIntensity();
                }
            }
        }
        return 0.0f;
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
};

// Global instance
WebSimulator g_simulator;

// Function to be called every frame
void main_loop() {
    g_simulator.update();
}

// JavaScript callback for changing scenes
extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void change_scene(int scene_index) {
        g_simulator.setScene(scene_index);
    }
    
    EMSCRIPTEN_KEEPALIVE
    int get_scene_count() {
        return g_simulator.getSceneCount();
    }
    
    EMSCRIPTEN_KEEPALIVE
    void set_brightness(uint8_t brightness) {
        g_simulator.setBrightness(brightness);
    }
    
    // Expose rotation functions to JavaScript
    EMSCRIPTEN_KEEPALIVE
    void update_rotation(float delta_x, float delta_y) {
        g_simulator.updateRotation(delta_x, delta_y);
    }
    
    EMSCRIPTEN_KEEPALIVE
    void reset_rotation() {
        g_simulator.resetRotation();
    }
    
    // New functions for auto-rotation and preset views
    EMSCRIPTEN_KEEPALIVE
    void set_auto_rotation(bool enabled, float speed) {
        g_simulator.setAutoRotation(enabled, speed);
    }
    
    EMSCRIPTEN_KEEPALIVE
    void set_preset_view(int preset_index) {
        g_simulator.setPresetView(preset_index);
    }
    
    EMSCRIPTEN_KEEPALIVE
    void set_zoom_level(int zoom_level) {
        g_simulator.setZoomLevel(zoom_level);
    }
    
    EMSCRIPTEN_KEEPALIVE
    void show_benchmark_report() {
        g_simulator.showBenchmarkReport();
    }
    
    EMSCRIPTEN_KEEPALIVE
    void toggle_debug_mode() {
        g_simulator.toggleDebugMode();
    }
    
    EMSCRIPTEN_KEEPALIVE
    void print_model_info() {
        g_simulator.printModelInfo();
    }
    
    // Expose scene management functions
    EMSCRIPTEN_KEEPALIVE
    int get_num_scenes() {
        return g_simulator.getSceneCount();
    }
    
    EMSCRIPTEN_KEEPALIVE
    void get_scene_name(int scene_index, char* buffer, int buffer_size) {
        g_simulator.getSceneName(scene_index, buffer, buffer_size);
    }
    
    EMSCRIPTEN_KEEPALIVE
    void set_scene(int scene_index) {
        g_simulator.setScene(scene_index);
    }
    
    // LED size and bloom intensity controls
    EMSCRIPTEN_KEEPALIVE
    void set_led_size(float size) {
        g_simulator.setLEDSize(size);
    }
    
    EMSCRIPTEN_KEEPALIVE
    float get_led_size() {
        return g_simulator.getLEDSize();
    }
    
    EMSCRIPTEN_KEEPALIVE
    void set_bloom_intensity(float intensity) {
        g_simulator.setBloomIntensity(intensity);
    }
    
    EMSCRIPTEN_KEEPALIVE
    float get_bloom_intensity() {
        return g_simulator.getBloomIntensity();
    }
    
    // Add functions for model stats
    EMSCRIPTEN_KEEPALIVE
    int get_led_count() {
        return g_simulator.getLEDCount();
    }
    
    EMSCRIPTEN_KEEPALIVE
    float get_fps() {
        return g_simulator.getFPS();
    }
    
    // Function to be called from JavaScript for logging
    EMSCRIPTEN_KEEPALIVE
    void log_message(const char* message) {
        PixelTheater::Log::warning("%s", message);
    }
    
    // JavaScript callback for getting brightness
    EMSCRIPTEN_KEEPALIVE
    uint8_t get_brightness() {
        return g_simulator.getBrightness();
    }
}

int main() {
    std::cout << "Initializing WebGL LED Simulator..." << std::endl;
    
    // Initialize the simulator
    if (!g_simulator.initialize()) {
        std::cerr << "Failed to initialize simulator" << std::endl;
        return 1;
    }
    
    // Set up the main loop
    std::cout << "Setting up main loop" << std::endl;
    emscripten_set_main_loop(main_loop, 0, 1);
    
    // This code will never be reached in Emscripten
    return 0;
}
#endif 