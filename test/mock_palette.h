#pragma once

// Mock FastLED palette for testing
class CRGBPalette16 {
public:
    CRGBPalette16() = default;
    // Add state for testing
    int state = 0;
}; 