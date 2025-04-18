#pragma once

#include <cstdint>
#include <memory> // For potential future use with unique_ptr if needed directly
#include <string> // Included for consistency, though not directly used in header now
#include "PixelTheater/core/crgb.h" // Need CRGB definition

namespace Scenes {

// Forward declaration of the scene class that owns Blobs
class BlobScene; 

// Blob class 
class Blob {
public:
    int sphere_radius = 100;  
    BlobScene& scene; // Reference to the parent scene for accessing model/utils
    uint16_t blob_id = 0;
    int radius = 0;      
    float a = 0.0f, c = 0.0f; // Position (angular coordinates)
    float av = 0.0f, cv = 0.0f; // Velocity (angular)
    float max_accel = 0.01f;  
    int age = 0;
    int lifespan = 1000;
    PixelTheater::CRGB color; // Use qualified type
    
    // Constructor
    Blob(BlobScene& parent_scene, uint16_t unique_id, int min_r, int max_r, int max_a, float speed);
    
    // Methods
    void estimateSphereRadius();
    void reset();
    int x() const; // Position (Cartesian) - calculated
    int y() const; // Position (Cartesian) - calculated
    int z() const; // Position (Cartesian) - calculated
    void applyForce(float af, float cf); // Apply force (angular)
    void applyForce(float fx, float fy, float fz); // Apply force (Cartesian)
    void tick(); // Update state
    
private:
    int min_radius;
    int max_radius;
    int max_age;
    float speed_scale;
};

} // namespace Scenes 