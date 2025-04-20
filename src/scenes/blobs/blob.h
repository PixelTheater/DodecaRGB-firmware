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
    // --- Initialized Members (Public Part) ---
    // Declaration order must match initializer list in blob.cpp
    BlobScene& scene;         // Initialized 1st
    uint16_t blob_id;     // Initialized 2nd
    // color will be declared after the private members it follows in the init list
    
    // --- Other public members (state, calculated, etc.) ---
    int radius = 0;           
    float a = 0.0f, c = 0.0f; 
    float av = 0.0f, cv = 0.0f; 
    float max_accel = 0.01f;  
    int age = 0;              
    int lifespan = 1000;      
    
    // Constructor
    Blob(BlobScene& parent_scene, uint16_t unique_id, int min_r, int max_r, int max_a, float speed);
    
    // Methods
    void reset();
    int x() const; 
    int y() const; 
    int z() const; 
    void applyForce(float af, float cf); 
    void applyForce(float fx, float fy, float fz); 
    void tick(); 
    
private:
    // --- Initialized Members (Private Part) ---
    // Declaration order must match initializer list in blob.cpp
    int min_radius;           // Initialized 3rd
    int max_radius;           // Initialized 4th
    int max_age;              // Initialized 5th
    float speed_scale;        // Initialized 6th

public: 
    // --- Initialized Members (Public Part continued) ---
    PixelTheater::CRGB color; // Initialized 7th

};

} // namespace Scenes 