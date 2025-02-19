#pragma once
#include "PixelTheater/model_def.h"
#include "PixelTheater/face.h"

namespace PixelTheater {
namespace Fixtures {

// Model for testing region operations
struct RegionTestModel : public ModelDefinition<40, 2> {
    // Required metadata
    static constexpr const char* NAME = "Region Test Model";
    static constexpr const char* VERSION = "1.0";
    static constexpr const char* DESCRIPTION = "Model for testing region operations";
    
    // Face types defined directly
    static constexpr FaceType FACE_TYPES[FACE_COUNT] = {
        FaceType::Pentagon,
        FaceType::Pentagon
    };
};

} // namespace Fixtures
} // namespace PixelTheater 