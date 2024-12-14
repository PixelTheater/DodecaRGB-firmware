#pragma once
#ifndef POINTS_H
#define POINTS_H

#include <Arduino.h>
#include <vector>
#include <algorithm>
#include <ArduinoEigen.h>
#include "points.h"

using namespace Eigen;

#define LEDS_PER_SIDE 104
#define NUM_SIDES 12
#define NUM_LEDS NUM_SIDES*LEDS_PER_SIDE 
#define MAX_LED_NEIGHBORS 7

struct distance_map {
    int led_number;
    float distance;
    Vector3d direction;
};

// define X,Y,Z struct
class LED_Point {
  public:
    int index;
    float x;
    float y;
    float z;
    //Vector3d pos;
    int side;       // which of the 12 sides it's on
    int label_num;  // which LED on the side (they are labelled)
  
    std::vector<distance_map> neighbors;
    void find_nearest_leds();
    float distance_to(float x, float y, float z);
    float distance_to(LED_Point *p);
    float distanceFrom(float x, float y, float z);

    // default constructor
    LED_Point(
      int i, float fx, float fy, float fz, 
      int fside, int flabel_num) : 
        index(i), x(fx), y(fy), z(fz), side(fside), label_num(flabel_num) { }
  
};

extern LED_Point points[];

extern bool compare_distance(distance_map a, distance_map b);
extern float calculatePointDistance(LED_Point p1, LED_Point p2);

#endif /* LED_POINTS_H_ */