#pragma once

#include "PixelTheater.h" 
#include "benchmark.h"
#include "blob.h" // Include the extracted Blob class definition

#include <vector>
#include <memory>
#include <cmath>
#include <algorithm> // For std::max, std::min, std::clamp
#include <string> // For status method (if restored)

// using namespace PixelTheater; // Avoid top-level using
// using namespace PixelTheater::Constants; // Avoid top-level using

namespace Scenes {

// Forward declaration
class BlobScene;

// BlobScene class
class BlobScene : public PixelTheater::Scene { 
public:
    BlobScene() = default; 
    
    // Static constants for default parameters
    static constexpr int DEFAULT_NUM_BLOBS = 15;
    static constexpr int DEFAULT_MIN_RADIUS = 70;
    static constexpr int DEFAULT_MAX_RADIUS = 120;
    static constexpr int DEFAULT_MAX_AGE = 4000;
    static constexpr float DEFAULT_SPEED = 0.2;
    static constexpr uint8_t DEFAULT_FADE = 10;
    
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