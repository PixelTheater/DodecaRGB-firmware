#pragma once

#include <cstdint>
#include <cstddef>

namespace Calibration {

// Identity domains (see runtime-config plan). Display uses 1-based BoardLabel.
using BoardLabel = uint8_t;      // 1..N
using WiringIndex = uint8_t;     // 0..N-1 addressable PCB order
using SlotIndex = uint8_t;       // 0..N-1 geometric slot
using RotationStep = uint8_t;    // 0..orientationCount-1

static constexpr size_t kFaceCount = 12;
static constexpr size_t kLedsPerFace = 135;
static constexpr size_t kOrientationCount = 5; // pentagon
static constexpr size_t kEdgeCount = 5;

struct FaceAssignment {
    BoardLabel boardLabel = 1;
    SlotIndex slotIndex = 0;
    RotationStep rotationStep = 0;
};

// Factory defaults match current DodecaRGBv2_1 model.yaml rotations
// (wiring order = faces list order; slotIndex = geometric_id = index).
inline void initFactoryAssignments(FaceAssignment* out) {
    static constexpr RotationStep kRot[kFaceCount] = {
        3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 0
    };
    for (size_t w = 0; w < kFaceCount; ++w) {
        out[w].boardLabel = static_cast<BoardLabel>(w + 1);
        out[w].slotIndex = static_cast<SlotIndex>(w);
        out[w].rotationStep = kRot[w];
    }
}

inline uint16_t wiringLedOffset(WiringIndex w) {
    return static_cast<uint16_t>(w) * static_cast<uint16_t>(kLedsPerFace);
}

} // namespace Calibration
