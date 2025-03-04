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
    std::unique_ptr<PixelTheater::WebGL::WebPlatform> platform;
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
    
    // View presets
    void setPresetView(int preset_index) {
        if (stage) {
            auto* platform = stage->getPlatform();
            if (platform) {
                auto* web_platform = dynamic_cast<PixelTheater::WebGL::WebPlatform*>(platform);
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
void set_preset_view(int preset_index) {
    if (g_simulator) {
        g_simulator->setPresetView(preset_index);
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
void get_scene_name(int scene_index, char* buffer, int buffer_size) {
    if (g_simulator) {
        g_simulator->getSceneName(scene_index, buffer, buffer_size);
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

#endif // defined(PLATFORM_WEB) || defined(EMSCRIPTEN) 