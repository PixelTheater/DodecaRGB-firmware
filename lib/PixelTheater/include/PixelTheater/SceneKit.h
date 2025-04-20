#pragma once

// SceneKit — convenience header for writing scenes
// ------------------------------------------------
// This header pulls the most frequently used PixelTheater symbols into
// namespace Scenes so that scene source files can use short, unqualified
// names (Scene, CRGB, PT_PI, etc.) while still keeping the global and
// PixelTheater namespaces clean.
//
// IMPORTANT:  SceneKit purposefully imports only high‑level *engine* API
// types and helpers.  It does NOT pull in asset‑specific data structures
// (e.g. TextureData) so individual scenes include the assets they need.
//
// Usage:
//   #include "PixelTheater/SceneKit.h"
//   #include "textures/texture_data.h"   // if your scene needs textures
//   namespace Scenes {
//     class MyScene : public Scene { … };
//   }

#include "PixelTheater.h"

namespace Scenes {

// ─── Base class ────────────────────────────────────────────────────────────
using PixelTheater::Scene;

// ─── Colour / palette types ────────────────────────────────────────────────
using PixelTheater::CRGB;
using PixelTheater::CHSV;
using PixelTheater::CRGBPalette16;

// ─── Utility helpers ───────────────────────────────────────────────────────
using PixelTheater::colorFromPalette;
using PixelTheater::map;  // Arduino‑style map() for int & float

// ─── Math constants ────────────────────────────────────────────────────────
using PixelTheater::Constants::PT_PI;
using PixelTheater::Constants::PT_TWO_PI;
using PixelTheater::Constants::PT_HALF_PI;

// ─── Wrapper helper for scale8_video (no FastLED global) ──────────────────
inline uint8_t scale8_video(uint8_t i, uint8_t scale) {
    return PixelTheater::scale8_video(i, scale);
}

// Bring common helpers directly into Scenes
using PixelTheater::fadeToBlackBy;
using PixelTheater::nblend;
using PixelTheater::blend8;
using PixelTheater::blend;

} // namespace Scenes 