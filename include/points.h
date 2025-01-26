#pragma once
#ifndef POINTS_H
#define POINTS_H

#include <Arduino.h>
#include <vector>
#include <algorithm>
#include <ArduinoEigen.h>

using namespace Eigen;

#define LEDS_PER_SIDE 104
#define NUM_SIDES 12
#define NUM_LEDS NUM_SIDES*LEDS_PER_SIDE 
#define MAX_LED_NEIGHBORS 7

struct neighbor_data {
    int led_number;
    float distance;
};

struct distance_map {
    int led_number;
    float distance;
    Vector3d direction;
};

// define X,Y,Z struct
class LED_Point {
  private:
    void init_neighbors(const std::vector<neighbor_data>& neighbors_init);
    
  public:
    int index;
    float x;
    float y;
    float z;
    uint8_t face;           // Which pentagon face (0-11)
    uint8_t face_index;     // Index within face (0-103)
    std::vector<Neighbor> neighbors;  // Physical wiring neighbors
  
    int side;       // which of the 12 sides it's on
    int label_num;  // which LED on the side (they are labelled)
  
    std::vector<distance_map> neighbors_map;

    // Constructor with neighbors data
    LED_Point(
      int i, float fx, float fy, float fz, 
      int fside, int flabel_num,
      const std::vector<neighbor_data>& neighbors_init = std::vector<neighbor_data>()) : 
        index(i), x(fx), y(fy), z(fz), 
        side(fside), label_num(flabel_num) {
          if (!neighbors_init.empty()) {
            init_neighbors(neighbors_init);
          }
        }

    void find_nearest_leds();  // Will exit if neighbors already initialized
    float distance_to(float x, float y, float z);
    float distance_to(LED_Point *p);
    float distanceFrom(float x, float y, float z);

    // Hardware-specific helpers
    int16_t distanceTo(const LED_Point& other) const;
    bool isNeighbor(uint16_t led_number) const;
};

extern LED_Point points[];

extern bool compare_distance(distance_map a, distance_map b);
extern float calculatePointDistance(LED_Point p1, LED_Point p2);

#endif /* POINTS_H_ */