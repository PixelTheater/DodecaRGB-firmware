#pragma once
#include <cstdint>

#ifdef TEST_BUILD
#include "mock_fastled.h"  // Use the mock version from test/helpers
#else
#include <FastLED.h>
#endif 

namespace Animation {

// Color utilities and palette management
class ColorUtils {
    // ... implementation
};

} // namespace Animation 