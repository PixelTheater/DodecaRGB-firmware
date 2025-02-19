#pragma once
#include "PixelTheater/model_def.h"
#include "PixelTheater/face.h"

namespace PixelTheater {
namespace Fixtures {

// A valid pentagon model with pre-configured regions
struct ValidPentagonModel : public ModelDefinition<40, 2> {
    // Model metadata
    static constexpr char NAME[] = "Valid Pentagon Model";
    static constexpr char VERSION[] = "1.0";
    static constexpr char DESCRIPTION[] = "Pre-configured pentagon model with valid regions";

 
};

} // namespace Fixtures
} // namespace PixelTheater 