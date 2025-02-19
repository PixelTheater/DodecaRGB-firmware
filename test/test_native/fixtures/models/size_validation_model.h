#pragma once
#include "PixelTheater/model_def.h"
#include "PixelTheater/face.h"

namespace PixelTheater {
namespace Fixtures {

// Large model for testing size limits
struct SizeValidationModel : public ModelDefinition<1000, 8> {
    // Required metadata
    static constexpr const char* NAME = "Size Validation Model";
    static constexpr const char* VERSION = "1.0";
    static constexpr const char* DESCRIPTION = "Large model for testing size limits";
    
    // Face definitions - direct array
    static constexpr FaceType FACE_TYPES[FACE_COUNT] = {
        FaceType::Pentagon, FaceType::Pentagon,
        FaceType::Pentagon, FaceType::Pentagon,
        FaceType::Pentagon, FaceType::Pentagon,
        FaceType::Pentagon, FaceType::Pentagon
    };
};

} // namespace Fixtures
} // namespace PixelTheater 