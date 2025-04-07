#pragma once

/**
 * @file PixelTheater.h
 * @brief Main include file for the PixelTheater library.
 * 
 * Include this file to use the core features of the library,
 * including defining Scenes and setting up the Theater.
 */

// Core Scene Definition
#include "PixelTheater/scene.h"

// Core Data Types
#include "PixelTheater/core/crgb.h"
#include "PixelTheater/model/point.h"
#include "PixelTheater/model/face.h"

// Interfaces (primarily for reference, not direct user interaction)
#include "PixelTheater/core/imodel.h"
#include "PixelTheater/core/iled_buffer.h"

// Platform Abstraction (may not be needed directly by user if using Theater)
#include "PixelTheater/platform/platform.h"

// Common Utilities & Helpers
#include "PixelTheater/core/color.h" 
#include "PixelTheater/core/math.h"
#include "PixelTheater/core/log.h"   // If Log:: or global logging is used
#include "PixelTheater/core/time.h"  // If global time functions are used
// #include "PixelTheater/core/constants.h" // If constants are needed

// Theater Facade (Once implemented)
#include "PixelTheater/theater.h" 

// Namespace alias for convenience (optional for user)
// namespace pt = PixelTheater; 