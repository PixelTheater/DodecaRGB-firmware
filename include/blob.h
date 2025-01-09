#pragma once
#ifndef BLOB_H
#define BLOB_H
#include <FastLED.h>

class Blob {
  public:
    int16_t blob_id = 0;
    const int sphere_r = 317; // radius of sphere the blob orbits
    const int min_radius = 100;
    const int max_radius = 130;

    int radius; // radius of blob
    float a,c = 0;  // polar angles
    float av;  // velocity of angles in radians
    float cv;  // velocity of angles in radians
    float max_accel = 0.0; // 0.17

    long max_age = 4000;
    long age;
    long lifespan;

    CRGB color = CRGB::White;

    // constructor
    Blob(uint16_t unique_id);

    void reset();
    int x();
    int y();
    int z();
    void applyForce(float a, float c);
    void applyForce(float x, float y, float z);
    void tick();

};

#endif /* BLOB_H_ */