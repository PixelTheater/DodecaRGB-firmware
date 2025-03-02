#include "PixelTheater/core/math.h"

namespace Scenes {

// Implementation of the math provider accessor
PixelTheater::MathProvider& getMathProvider() {
    // Use a static instance of DefaultMathProvider
    static PixelTheater::DefaultMathProvider provider;
    return provider;
}

} // namespace Scenes 