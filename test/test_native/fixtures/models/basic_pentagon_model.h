#pragma once
#include "PixelTheater/model_def.h"
#include "PixelTheater/face.h"

namespace PixelTheater {
namespace Fixtures {

// Basic model with two pentagon faces for testing core functionality
struct BasicPentagonModel : public ModelDefinition<40, 2> {
    static constexpr const char* NAME = "Basic Pentagon Model";
    static constexpr const char* VERSION = "1.0";
    static constexpr const char* DESCRIPTION = "Two pentagon faces for basic tests";
    
    static constexpr FaceType FACE_TYPES[FACE_COUNT] = {
        FaceType::Pentagon,
        FaceType::Pentagon
    };
};

} // namespace Fixtures
} // namespace PixelTheater 