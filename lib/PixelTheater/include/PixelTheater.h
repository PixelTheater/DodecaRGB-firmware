#pragma once

/**
 * @file PixelTheater.h
 * @brief Main include file for the PixelTheater library.
 * 
 * Include this file to use the core features of the library,
 * including defining Scenes and setting up the Theater.
 */

// --- Core --- 
#include "PixelTheater/platform/platform.h" // Platform Abstraction (time, random, log)
#include "PixelTheater/scene.h"      // Base Scene class
#include "PixelTheater/core/crgb.h"           // CRGB color struct
#include "PixelTheater/core/math_utils.h"     // Math utilities (lerp, etc.)
#include "PixelTheater/constants.h"      // Constants (PI, TWO_PI)

// --- Model --- 
#include "PixelTheater/model/point.h"         // Point struct
#include "PixelTheater/model/face.h"          // Face struct
#include "PixelTheater/model/model.h"         // Model class

// --- Theater --- 
#include "PixelTheater/theater.h"      // Main Theater controller class

// --- Color API --- 
#include "PixelTheater/color/palettes.h"    // Standard Palettes enum & data
#include "PixelTheater/color/gradients.h"   // Generated Gradient palette data
#include "PixelTheater/color/palette_api.h" // Core palette functions (colorFromPalette, blend)
#include "PixelTheater/color/definitions.h" // Static color definitions (CRGB::Red, etc.)
#include "PixelTheater/color/conversions.h" // Color conversion functions (rgb2hsv, hsv2rgb)
#include "PixelTheater/color/measurement.h" // Color measurement functions (distance, brightness)
#include "PixelTheater/color/identity.h"    // Color identity functions (name, ANSI)
#include "PixelTheater/color/fill.h"        // Fill functions (fill_solid, fill_gradient)
