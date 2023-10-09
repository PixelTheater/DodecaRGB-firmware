#pragma once

#ifndef PARTICLE_H
#define PARTICLE_H

#include <vector>
#include <algorithm>
#include "points.h"

extern LED_Point points[];

#define MAX_PATH_LENGTH 11
#define PARTICLE_HOLD_TIME 10

class Particle {
    enum LedStatus {free, held};
    int sphere_r = 310; // radius of sphere the blob orbits

    public:
        int led_number;
        uint32_t color;
        int age;       
        std::vector<int> path; 
        int hold_time;
        LedStatus status;
        float a,c = 0;  // polar angles
        float av;  // velocity of angles in radians
        float cv;  // velocity of angles in radians

        float x();
        float y();
        float z();
        
        void reset();
        void tick();

        Particle();

};

#endif