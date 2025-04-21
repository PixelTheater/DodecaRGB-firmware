#pragma once

#include "PixelTheater/SceneKit.h" 
#include "benchmark.h"
#include "blob.h" // Individual Blob class definition

#include <vector>
#include <memory>

namespace Scenes {

// Forward declaration
class BlobScene;

// BlobScene class
class BlobScene : public Scene { 
public:
    BlobScene() = default; 
    
    // Static constants for default parameters
    static constexpr int DEFAULT_NUM_BLOBS = 8;
    static constexpr int DEFAULT_MIN_RADIUS = 70;
    static constexpr int DEFAULT_MAX_RADIUS = 130;
    static constexpr int DEFAULT_MAX_AGE = 4000;
    static constexpr float DEFAULT_SPEED = 0.25f;
    static constexpr uint8_t DEFAULT_FADE = 8; // Reverted type to uint8_t
    static constexpr int FADE_IN_DURATION = 150; // Frames for fade-in
    
    // Scene lifecycle methods (Declarations only)
    void setup() override;
    void tick() override;
    // void reset() override; // Optional: Add declaration if needed

    // Scene-specific logic (Declarations only)
    void initBlobs();
    void updateBlobs();
    void drawBlobs();
    
    friend class Blob;

private:
    std::vector<std::unique_ptr<Blob>> blobs;
};

// Implementations moved to blob_scene.cpp

} // namespace Scenes 