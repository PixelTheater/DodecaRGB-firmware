#include "particle.h"

Particle::Particle() {
    this->reset();
}

float Particle::x(){ return sphere_r * sin(this->c)*cos(this->a); }
float Particle::y(){ return sphere_r * sin(this->c)*sin(this->a); }
float Particle::z(){ return sphere_r * cos(this->c); }

void Particle::reset() {
    this->led_number = 1;
    this->color = 255*255 + random(200);
    this->age = 0;
    this->a = 0;
    this->c = random(TWO_PI*1000)/1000.0;
    this->av = random(30, 90)/5000.0;
    this->cv = random(30, 90)/500.0;
    this->hold_time = random(PARTICLE_HOLD_TIME,PARTICLE_HOLD_TIME+10);
    this->status = free;
}


void Particle::tick() {
    this->age++;
    if (this->age > this->hold_time) {
        this->a += av;
        this->c += cv;

        this->status = held;        
        LED_Point *p = &points[this->led_number];
        
        int best_pick = -1;
        for (int i=1; i<MAX_LED_NEIGHBORS; i++) {
            distance_map candidate = p->neighbors.at(i);
            if (best_pick == -1) { // check if best_pick is null
                best_pick = candidate.led_number;
                //Serial.printf("best_pick is null, setting to %d\n", best_pick);
            }
            float candidate_distance = points[candidate.led_number].distance_to(x(), y(), z());
            float best_pick_distance = points[best_pick].distance_to(x(), y(), z());
            //Serial.printf("candidate_distance: %f, best_pick_distance: %f\n", candidate_distance, best_pick_distance);
            //Serial.printf("i: %d, candidate: %d, best_pick: %d, distance: %f\n", i, candidate.led_number, best_pick, candidate_distance);
            if (candidate_distance < best_pick_distance) {
                best_pick = candidate.led_number;
                //Serial.printf("best_pick is now %d, better than %d\n", best_pick, best_pick_distance);
            }
        }    
        // next_led = p->neighbors.at(random(MAX_LED_NEIGHBORS)).led_number; 
        this->led_number = best_pick;
        this->path.insert(this->path.begin(), this->led_number);
        this->path.resize(MAX_PATH_LENGTH);
        this->age = 0;
    }
}

