#pragma once
#include <FastLED.h>

class Blob {
  public:
    int sphere_r = 290; // radius of sphere the blob orbits
    int radius; // radius of blob
    int16_t a,c = 0;  // polar angles
    int16_t av;  // velocity of angles in radians
    int16_t cv;  // velocity of angles in radians
    int16_t max_accel = 25500;
    long age;
    long lifespan;

    CRGB color;

    // constructor
    Blob();

    void reset();
    int x();
    int y();
    int z();
    void applyForce(float a, float c);
    void tick();

};

