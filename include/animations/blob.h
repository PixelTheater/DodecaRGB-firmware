#pragma once
#include "animation.h"
#include "palettes.h"

class Blob {
public:
    static const int sphere_r = 317;  // radius of sphere the blob orbits
    int16_t blob_id = 0;
    int radius;      // radius of blob
    float a, c = 0;  // polar angles
    float av, cv;    // velocity of angles in radians
    float max_accel = 0.0;  // 0.17

    long age;
    long lifespan;

    CRGB color = CRGB::White;

    Blob(uint16_t unique_id, int min_r, int max_r, long max_a, float speed);
    void reset();
    int x();
    int y();
    int z();
    void applyForce(float a, float c);
    void applyForce(float x, float y, float z);
    void tick();

private:
    int min_radius;
    int max_radius;
    long max_age;
    float speed_scale;
};

class BlobAnimation : public Animation {
private:
    int num_blobs;
    int min_radius;
    int max_radius;
    long max_age;
    float speed;  // Scale factor for max_accel
    std::vector<std::unique_ptr<Blob>> blobs;
    uint8_t fade_amount;

public:
    // Default values
    static constexpr int DEFAULT_NUM_BLOBS = 7;
    static constexpr int DEFAULT_MIN_RADIUS = 100;
    static constexpr int DEFAULT_MAX_RADIUS = 130;
    static constexpr long DEFAULT_MAX_AGE = 4000;
    static constexpr float DEFAULT_SPEED = 1.0;
    static constexpr uint8_t DEFAULT_FADE = 10;

    void init(const AnimParams& params) override;
    void tick() override;
    String getStatus() const override;
};