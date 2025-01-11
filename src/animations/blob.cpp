#include "animations/blob.h"
#include "points.h"

/* 
Blobs are points that orbit a sphere at given radius. 
As they orbit, they leave a trail of color behind them.
Each blob has a color, and a radius.
They also have velocity in the A and C angles.
Every loop, the angles are updated by the velocity by calling the tick() function
*/

Blob::Blob(uint16_t unique_id, int min_r, int max_r, long max_a, float speed) 
    : blob_id(unique_id),
      min_radius(min_r),
      max_radius(max_r),
      max_age(max_a),
      speed_scale(speed) {
    this->reset();
}

void Blob::reset() {
    this->age = 0;
    this->lifespan = random(this->max_age/2) + this->max_age/2;
    this->radius = random(this->min_radius, this->max_radius);
    this->max_accel = random(5,27)/1000.0 * speed_scale;  // Apply speed scaling
    this->av = random(-this->max_accel * 1000, this->max_accel * 1000) / 1000.0;
    this->cv = random(-this->max_accel * 1000, this->max_accel * 1000) / 1000.0;
    this->a = random(TWO_PI*1000)/1000.0 - PI;
    this->c = random(TWO_PI*10000)/10000.0 - PI;
}

int Blob::x() { return sphere_r * sin(this->c) * cos(this->a); }
int Blob::y() { return sphere_r * sin(this->c) * sin(this->a); }
int Blob::z() { return sphere_r * cos(this->c); }

void Blob::applyForce(float af, float cf) {
    this->av += af;
    if (this->av > this->max_accel) this->av = this->max_accel;
    if (this->av < -this->max_accel) this->av = -this->max_accel;
    this->cv += cf;
    if (this->cv > this->max_accel) this->cv = this->max_accel;
    if (this->cv < -this->max_accel) this->cv = -this->max_accel;
}

void Blob::applyForce(float fx, float fy, float fz) {
    float af = atan2(fy, fx);
    float cf = atan2(sqrt(fx*fx + fy*fy), fz);
    this->applyForce(af, cf);
}

void Blob::tick() {
    float force_av = this->av * 1.001;
    this->c = fmod(this->c + PI, TWO_PI) - PI;  // Normalize c to be within [-PI, PI]
    float force_cv = 0.00035 * (this->c - PI/2);
    if (this->c < -PI/2) {
        force_cv = -0.0003 * (this->c + PI/2);
    }
    this->applyForce(force_av, force_cv);

    this->age++;
    this->av *= 0.99; 
    this->cv *= 0.99; 
    this->a += this->av; 
    this->c += this->cv;
    
    if (abs(this->cv) < 0.001) {
        float af = random(-max_accel*1000, max_accel*1000);
        float cf = random(-max_accel*1000, max_accel*1000);
        this->applyForce(af/2000.0, cf/1000.0);
    }
    
    if (this->lifespan - this->age < max_age/20) {
        this->radius *= 0.99;
    }
    if (this->age > this->lifespan) {
        this->reset();
    }
}

void BlobAnimation::init(const AnimParams& params) {
    Animation::init(params);
    
    // Get parameters with defaults
    num_blobs = params.getInt("num_blobs", DEFAULT_NUM_BLOBS);
    min_radius = params.getInt("min_radius", DEFAULT_MIN_RADIUS);
    max_radius = params.getInt("max_radius", DEFAULT_MAX_RADIUS);
    max_age = params.getInt("max_age", DEFAULT_MAX_AGE);
    speed = params.getFloat("speed", DEFAULT_SPEED);
    fade_amount = params.getInt("fade", DEFAULT_FADE);
    
    // Get palette for colors
    const CRGBPalette16 palette = params.getPalette("palette", RainbowColors_p);  // Default to rainbow if no palette specified
    
    // Create blobs with unique colors from palette
    blobs.clear();
    for (int i = 0; i < num_blobs; i++) {
        auto blob = std::make_unique<Blob>(i, min_radius, max_radius, max_age, speed);
        blob->color = ColorFromPalette(palette, i * (256 / num_blobs));  // Evenly space colors
        blobs.push_back(std::move(blob));
    }
}

void BlobAnimation::tick() {
    static const float forceStrength = 0.000005;  // Tuning variable for repelling force
    
    // Update and draw blobs
    for (auto& blob : blobs) {
        blob->tick();
        
        // Draw blob effect on LEDs
        auto rad_sq = blob->radius * blob->radius;
        for (int i = 0; i < numLeds(); i++) {
            int dx = points[i].x - blob->x();
            int dy = points[i].y - blob->y();
            int dz = points[i].z - blob->z();
            int dist = dx*dx + dy*dy + dz*dz;
            
            if (dist < rad_sq) {
                CRGB c = blob->color;
                if (blob->age < 150) {
                    c.fadeToBlackBy(map(blob->age, 0, 150, 180, 1));
                }
                nblend(leds[i], c, map(dist, 0, rad_sq, 7, 3));
            }
        }
    }
    
    // Apply repelling forces between blobs
    for (size_t i = 0; i < blobs.size(); i++) {
        for (size_t j = i + 1; j < blobs.size(); j++) {
            float min_dist = (blobs[i]->radius + blobs[j]->radius) / 2;
            float min_dist_sq = min_dist * min_dist;
            
            float dx = blobs[i]->x() - blobs[j]->x();
            float dy = blobs[i]->y() - blobs[j]->y();
            float dz = blobs[i]->z() - blobs[j]->z();
            float dist_sq = dx*dx + dy*dy + dz*dz;
            
            if (dist_sq < min_dist_sq && dist_sq > 20) {
                float dist = sqrt(dist_sq);
                float force = ((min_dist - dist) / min_dist) * forceStrength;
                
                // Normalize direction vector
                float nx = dx / dist;
                float ny = dy / dist;
                float nz = dz / dist;
                
                // Apply repelling forces
                blobs[i]->applyForce(nx * force, ny * force, nz * force);
                blobs[j]->applyForce(-nx * force, -ny * force, -nz * force);
            }
        }
    }
    
    // Fade all LEDs
    for (int i = 0; i < numLeds(); i++) {
        leds[i].fadeToBlackBy(fade_amount);
    }
}

String BlobAnimation::getStatus() const {
    output.printf("Blobs: %d active (speed=%.2f, fade=%d)\n", 
        num_blobs, speed, fade_amount);
    output.printf("Radius: %d-%d, MaxAge: %ld\n", min_radius, max_radius, max_age);
    
    for (const auto& blob : blobs) {
        output.print(getAnsiColorString(blob->color));
        output.printf(" Blob %d: age=%ld/%ld accel=%.2f/%.2f\n",
            blob->blob_id, blob->age, blob->lifespan, blob->av, blob->cv);
    }
    return output.get();
}