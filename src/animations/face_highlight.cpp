#include "animations/face_highlight.h"

void FaceHighlight::init(const AnimParams& params) {
    speed = params.getFloat("speed", 1.0f);
    palette = params.getPalette("palette", RainbowColors_p);
}

void FaceHighlight::tick() {
    // Update face selection every N frames
    if (counter % (int)(50/speed) == 0) {
        current_face = (current_face + 1) % num_sides;
    }

    // Set face colors
    for (int i = 0; i < numLeds(); i++) {
        int face = i / leds_per_side;
        if (face == current_face) {
            // Selected face LEDs are  white
            leds[i] = CRGB(128, 128, 128);
        } else {
            // Other LEDs are dimmed
            leds[i] = CRGB(16, 16, 16);
        }
    }

    counter++;
}

String FaceHighlight::getStatus() const {
    output.printf("Face: %d Speed: %.2f\n", current_face, speed);
    return output.get();
} 