#include "animations/geography.h"

void Geography::init(const AnimParams& params) {
    Animation::init(params);
    // Randomize Lorenz parameters slightly
    sigma += random(400)/100.0;
    rho += random(400)/100.0;
    beta += random(100)/100.0;
}

void Geography::tick() {
    // Calculate Lorenz system
    float dx = sigma * (y - x);
    float dy = x * (rho - z) - y;
    float dz = x * y - beta * z;

    x += dx * dt;
    y += dy * dt;
    z += dz * dt;

    // Normalize values
    float normalized_x = (x + 20.0) / 40.0 * 2.5 - 0.8;
    float normalized_y = (y + 30.0) / 40.0 * 2.0 - 0.4;
    float normalized_z = (z + 20.0) / 30.0 * 3.0 - 0.9;

    // Update all LEDs
    for (int i = 0; i < numLeds(); i++) {
        float a = acos(points[i].y / sphere_r);
        float c = atan2(points[i].z, points[i].x) + (16.0-spin_angle)*10;
        int c_start = map(a, 0, TWO_PI, 50, 200);
        int c_end = map(c, 0, PI, 80, 255);
        int hue = map(fmod(normalized_y/25.0+a+c+shift/15-cos(millis()/2000.0), 50), 0, 40, c_start, c_end);
        int brightness = map(sin(a*shift/6.0+c*cos(normalized_x/5.0)), -3.6, 5.3, 1, 210);
        leds[i] = CHSV(hue, 255, brightness);
    }

    // Update animation parameters
    spin_angle += spin_dir*0.005 + normalized_z/250.0;
    spin_dir = -spin_angle/8.0;
    shift = (normalized_z-2.0)*5.5;
}

String Geography::getStatus() const {
    output.printf("Spin: %.2f Shift: %.2f", spin_angle, shift);
    return output.get();
} 