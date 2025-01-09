#include <Arduino.h>
#include <Wire.h>
#include <InternalTemperature.h>
#include <cmath>
#define FASTLED_USES_OBJECTFLED
#include <FastLED.h>

#include "color_lookup.h"
#include "blob.h"
#include "points.h"
#include "particle.h"

#include <Adafruit_LSM6DSOX.h>

// Function declarations
float get_perceived_brightness(const CRGB& color);
float get_contrast_ratio(const CRGB& color1, const CRGB& color2);
float get_hue_distance(const CRGB& color1, const CRGB& color2);

/* 

# DodecaRBG V2
- We have a dodecahedron model with 12 sides. 
- Each side is a pentgon-shaped PCB circuit board with RGB LEDs arranged on them, spaced evenly. 
- A Teensy 4.1 microcontroller is used to control the LEDs, using the Arduino environment and FastLED library.
- Each pentagon side contains 104 RGB leds on a circuit board, arranged in rings from the center.
- Each side connects to the next, in series, for a grand total of 1247 LEDs. 
- Two hemispheres of the model are on separate channels, using pins 19 and 18 of the Teensy.
- FastLED parallel support is being used (see https://github.com/FastLED/FastLED/releases/tag/3.9.8)

## Configuration

As the PCB circuit boards are wired together to form the dodecahedron, the arrangement of the sides must be 
defined, as there are many possible configurations. In addition, the rotation of each side must be defined, 
as it can have five possible rotations.

There's a processing sketch at https://github.com/somebox/dodeca-rgb-simulator that generates the list
of points, the X,Y,Z coordinates, and defines the order of the sides and their rotations. It also
renders an interactive 3D model of the dodecahedron. To change your dodecahedron's configuration,
you will need to re-generate the point mapping using this tool.

## Change Log

v2.1 Jan 2025:
- added support for orientation sensor
- improved random seeding based on CPU temp and analog noise
- new animations: ...

v2.0 Dec 8 2024:
- new hardware, micro-leds (1615 package), 104 per side
- overall size is smaller, around 80% compared to v1
- switched to Teensy microcontroller, code refactoring in progress
- working, but only tested with 6 sides active (624) and frame rates of 50fps are achievable so far

v1.0 Aug 2023:
- version 1 with only 26 LEDs per side, 312 in total
- initial animations, including blobs, 3d lines, particles, and color show
- released at CCC Camp 2023 https://hackaday.io/project/192557-dodecargb

*/

// LED configs
#define VERSION "2.0.3"
#define USER_BUTTON 2
// https://github.com/FastLED/FastLED/wiki/Parallel-Output#parallel-output-on-the-teensy-4
// pins 19+18 are used to control two strips of 624 LEDs, for a total of 1248 LEDs
#define LED_CHANNEL_1_PIN 19  // Teensy 4.1/fastled pin order: .. ,19,18,14,15,17, ..
#define LED_CHANNEL_2_PIN 18  // Teensy 4.1/fastled pin order: .. ,19,18,14,15,17, ..
#define ANALOG_PIN_A 24
#define ANALOG_PIN_B 25
#define ON_BOARD_LED 13

// Teensy I2C on pins 17/16 (SDA,SCL) is mapped to Wire1 in Arduinio framework
// https://www.pjrc.com/teensy/td_libs_Wire.html
// ex: Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29, &Wire1);

#define BRIGHTNESS  40
#define WIFI_ENABLED false
#define USE_IMU true


// base color palette
#define NUM_COLORS 16

CRGBPalette16 basePalette = CRGBPalette16( 
  CRGB::Red, CRGB::DarkRed, CRGB::IndianRed, CRGB::OrangeRed,
  CRGB::Green, CRGB::DarkGreen, CRGB::LawnGreen, CRGB::ForestGreen,
  CRGB::Blue, CRGB::DarkBlue, CRGB::SkyBlue, CRGB::Indigo,
  CRGB::Purple, CRGB::Indigo, CRGB::CadetBlue, CRGB::AliceBlue
); 
CRGBPalette16 highlightPalette = CRGBPalette16(
  CRGB::Yellow, CRGB::LightSlateGray, CRGB::LightYellow, CRGB::LightCoral, 
  CRGB::GhostWhite, CRGB::LightPink, CRGB::AntiqueWhite, CRGB::LightSkyBlue, 
  CRGB::Gold, CRGB::PeachPuff, CRGB::FloralWhite, CRGB::PaleTurquoise, 
  CRGB::Orange, CRGB::MintCream, CRGB::FairyLightNCC, CRGB::LavenderBlush
);
CRGBPalette16 uniquePalette = CRGBPalette16(
  CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::Yellow, 
  CRGB::Purple, CRGB::Orange, CRGB::Cyan, CRGB::Magenta, 
  CRGB::Lime, CRGB::Pink, CRGB::Turquoise, CRGB::Sienna,
  CRGB::Gold, CRGB::Salmon, CRGB::Silver, CRGB::Violet
);

CRGB leds[NUM_LEDS];

Adafruit_LSM6DSOX sox;

// Constants
const int LEDS_PER_RING = 10;
const int LEDS_PER_EDGE = 15;

long random_seed = 0;
int mode = 0;

int show_pos = 0;
int show_color = random(255);

void color_show(){
  static int led_limit = 54;
  // turn off all LEDs in a dissolving pattern
  fadeToBlackBy(leds, NUM_LEDS, 2);
  
  // light up all LEDs in sequence
  for (int n=0; n<NUM_SIDES; n++){
    for (int i=0; i<led_limit; i++){
      int offset = sin8(millis()/((n+1)*400)) + cos8(n*(millis()/1000));
      int dist = abs(i - (show_pos+offset/10)) % led_limit;
      CRGB c = CHSV((show_color+i+offset)%255, 255, constrain(128-dist*4,0,128));
      nblend(leds[n*104+i], c, 50);
    }
  }
  FastLED.show();
  delay(1);
  show_pos++;
  if (show_pos > NUM_LEDS){
    show_pos = 0;
    show_color = random(255);
  }
}

void solid_sides(){
  static int s=0;
  if (random(12)==0) { s = random(NUM_SIDES); };
  CRGB c = CHSV(random(255), random(100)+150, random(255));
  for (int level=0; level<50; level+=1){
    for (int i=s*LEDS_PER_SIDE; i<(s+1)*LEDS_PER_SIDE; i++){
      nblend(leds[i], c, 10);
    }
    FastLED.show();
    if (digitalRead(USER_BUTTON) == LOW) return;
    delayMicroseconds(50);
  } 
  // for (int level=100; level>0; level--){
  //   for (int i=(s)*LEDS_PER_SIDE; i<(s+1)*LEDS_PER_SIDE; i++){
  //     nblend(leds[i], CHSV(millis()/random(100,1000) % 255, random(150)+100, random(120)+10), 3);
  //   }
  //   FastLED.show();
  //   if (digitalRead(USER_BUTTON) == LOW) return;
  //   //delayMicroseconds(100);
  // }
  // FastLED.show();
  s = (s+1) % NUM_SIDES;
}


CRGB c,c2;
// randomly light up LEDs and fade them out individually, like raindrops
uint8_t color_mix = 0;
static int period = 580;
int seed1,seed2;
uint8_t blend1;
uint8_t blend2;
int power_fade=1;
u_int8_t num_picks=1;

void flash_fade_points(){
  uint8_t color_warble = (int)(sin8_C(millis()/(period/11)) / 16);
  color_mix = (int)(64 + sin8_C(millis()/(period)+seed1+color_warble) / 1.5);
  // cycle color over time
  c = ColorFromPaletteExtended( basePalette, sin16_C(millis()/16+seed1*10), 255, LINEARBLEND);
  c2 = ColorFromPaletteExtended( highlightPalette, sin16_C((millis()/8+seed2*50)), 255, LINEARBLEND);
  blend1=sin8_C(millis()/(period*4.2)+seed1)/1.5+32;
  blend2=sin8_C(millis()/(period*3.5)+seed2)/2+32;
  for (int n=0; n<NUM_SIDES; n++){
    num_picks = map(power_fade,1,40,30,5);  // as the power level decreases, LEDs lit increases
    for (int m=0; m<num_picks; m++){
      if (random8(128) < color_mix) {
        int r1 = random(n*LEDS_PER_SIDE, (n+1)*LEDS_PER_SIDE);        
        nblend(leds[r1], c, map(blend1, 0, 255, 1, 7));
      }
      if (random8(128) < (256-color_mix)) {
        int r2 = random(n*LEDS_PER_SIDE, (n+1)*LEDS_PER_SIDE);
        nblend(leds[r2], c2, map(blend2, 0, 255, 1, 10));        
      }
    }
  }
  // fades
  int power = calculate_unscaled_power_mW(leds, NUM_LEDS);
  power_fade = (power_fade * 19 + max(map(power,8000,20000,1,40),1))/20;  
  for (int i = 0; i < NUM_LEDS; i++){
    int v = leds[i].getAverageLight();
    if (v > random8(power_fade/2)) {
      leds[i].fadeToBlackBy(random8(power_fade));
    }
  }
  //blur1d(leds, NUM_LEDS, map(sin8_C(millis()/300),0,256,0,128));
  //fadeToBlackBy(leds, NUM_LEDS, map(power, 6500, 18000, 1, 15));
  FastLED.show();   
  //delay(1);  
}

// Constants
#define SMOOTHING_FACTOR 0.005

// Variables for dynamic range calibration
int noiseMin = 120;
int noiseMax = 230;

// Smoothed value
float smoothedValue = 0.0;

float getSmoothNoise() {
    // Step 1: Read the analog signal
    int rawValue = analogRead(ANALOG_PIN_A);

    // Step 2: Update noise range dynamically
    if (rawValue < noiseMin) noiseMin = rawValue;
    if (rawValue > noiseMax) noiseMax = rawValue;

    // Prevent divide by zero
    int range = noiseMax - noiseMin;
    if (range == 0) range = 1;

    // Step 3: Map the raw value to the range [-1.0, 1.0]
    float mappedValue = (float)(rawValue - noiseMin) / range * 2.0 - 1.0;

    // Step 4: Apply smoothing
    smoothedValue += SMOOTHING_FACTOR * (mappedValue - smoothedValue);

    return smoothedValue;
}

void tv_static(){
  for (int i = 0; i<NUM_LEDS; i++){
    // Read the analog values
    int rawValueA = analogRead(ANALOG_PIN_A);
    int rawValueB = analogRead(ANALOG_PIN_B);

    // Update noise range dynamically for both analog pins
    static int noiseMinA = 1023, noiseMaxA = 0;
    static int noiseMinB = 1023, noiseMaxB = 0;

    if (rawValueA < noiseMinA) noiseMinA = rawValueA;
    if (rawValueA > noiseMaxA) noiseMaxA = rawValueA;
    if (rawValueB < noiseMinB) noiseMinB = rawValueB;
    if (rawValueB > noiseMaxB) noiseMaxB = rawValueB;

    // Prevent divide by zero
    int rangeA = noiseMaxA - noiseMinA;
    int rangeB = noiseMaxB - noiseMinB;
    if (rangeA == 0) rangeA = 1;
    if (rangeB == 0) rangeB = 1;

    // Map the raw values to the range [0, 255]
    int mappedValueA = (rawValueA - noiseMinA) * 255 / rangeA;
    int mappedValueB = (rawValueB - noiseMinB) * 255 / rangeB;

    // Set the LED color based on the mapped values
    leds[i] = CHSV(mappedValueA, 255, mappedValueB);
  }
  FastLED.show();
}

float spin_angle = 16.0;
float shift = 0;
float spin_dir = 0.0;
void geography_show(){
  static int sphere_r = 310; // radius of sphere

  // Lorenz parameters
  static float sigma = 8.0+random(400)/100.0;  // Prandtl number
  static float rho = 24.0+random(400)/100.0;    // Rayleigh number
  static float beta = 8.0 / 3+random(100)/100.0;

  // Time step
  static float dt = 0.002;

  // State variables
  static float x = 0.1, y = 0.3, z = -0.2;  // Initial conditions
  // Calculate the derivatives
  float dx = sigma * (y - x);
  float dy = x * (rho - z) - y;
  float dz = x * y - beta * z;

  // Update the state variables using Euler's method
  x += dx * dt;
  y += dy * dt;
  z += dz * dt;

  // Normalize x to the range [-1.0, 1.0]
  // You may also choose y or z for variation
  float normalized_x = (x + 20.0) / 40.0 * 2.5 - 0.8;  // Assuming x typically stays within [-20, 20]
  float normalized_y = (y + 30.0) / 40.0 * 2.0 - 0.4;  // Assuming x typically stays within [-20, 20]
  float normalized_z = (z + 20.0) / 30.0 * 3.0 - 0.9;  // Assuming x typically stays within [-20, 20]

  for (int i = 0; i<NUM_LEDS; i++){
    float a = acos(points[i].y / sphere_r);
    float c = atan2(points[i].z, points[i].x)+(16.0-spin_angle)*10;
    int c_start = map(a, 0, TWO_PI, 50, 200);
    int c_end = map(c, 0, PI, 80, 255);
    int hue = map(fmod(normalized_y/25.0+a+c+shift/15-cos(millis()/2000.0), 50), 0, 40, c_start, c_end);
    int brightness = map(sin(a*shift/6.0+c*cos(normalized_x/5.0)), -3.6, 5.3, 1, 210);
    leds[i] = CHSV(hue, 255, brightness);
  }
  FastLED.show();
  spin_angle += spin_dir*0.005+normalized_z/250.0;
  spin_dir = -spin_angle/8.0;
  //shift=getSmoothNoise()*5;
  shift = (normalized_z-2.0)*5.5;
}

int calculateBlobDistance(LED_Point p1, Blob *b) {
  int dx = p1.x - b->x();
  int dy = p1.y - b->y();
  int dz = p1.z - b->z();
  return (dx*dx + dy*dy + dz*dz);  
}

#define NUM_BLOBS 7
Blob *blobs[NUM_BLOBS];

void orbiting_blobs(){
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  sox.getEvent(&accel, &gyro, &temp);
  
  for (int b=0; b<NUM_BLOBS; b++){
    auto rad_sq = blobs[b]->radius*blobs[b]->radius;
    for (int i = 0; i<NUM_LEDS; i++){ 
      int dist = calculateBlobDistance(points[i], blobs[b]);
      if (dist < rad_sq){
        CRGB c = blobs[b]->color;
        // slow fade in
        if (blobs[b]->age < 150){
          c.fadeToBlackBy(map(blobs[b]->age, 0, 150, 180, 1));
        }
        nblend(leds[i], c, map(dist, 0, rad_sq, 7, 3));
      }
    }
  }

  // Tuning variable for repelling force strength
  static float forceStrength = 0.000005;

  // Apply repelling force between blobs
  for (int b1 = 0; b1 < NUM_BLOBS; b1++) {
    for (int b2 = b1 + 1; b2 < NUM_BLOBS; b2++) {
      float min_dist = (blobs[b1]->radius + blobs[b2]->radius)/2;
      float min_dist_sq = min_dist * min_dist;

      float dx = blobs[b1]->x() - blobs[b2]->x();
      float dy = blobs[b1]->y() - blobs[b2]->y();
      float dz = blobs[b1]->z() - blobs[b2]->z();
      float dist_sq = dx*dx + dy*dy + dz*dz;

      if (dist_sq < min_dist_sq and dist_sq > 20) {
        float dist = sqrt(dist_sq);
        float force = ((min_dist - dist) / min_dist) * forceStrength; // Repelling force based on distance
        //force += random(100)/100000.0; // Add a little randomness to the force

        // Normalize the direction vector
        float nx = dx / dist;
        float ny = dy / dist;
        float nz = dz / dist;

        // Apply the repelling force to each blob
        blobs[b1]->applyForce(nx * force, ny * force, nz * force);
        blobs[b2]->applyForce(-nx * force, -ny * force, -nz * force);
      }
    }
  }
  //fadeToBlackBy(leds, NUM_LEDS, 3);  
  for (int i=0; i<NUM_LEDS; i++){
    leds[i].fadeToBlackBy(10);
  }

  FastLED.show();

  for (int b=0; b<NUM_BLOBS; b++){
    blobs[b]->tick();
  }
}

void fade_test(){
  static float max_range = 500;
  static float zi = -max_range;
  static float yi = -max_range;
  static float xi = -max_range;
  static float target = 140;
  static int counter = 0;
  static int min_off = 0;
  float speed = 0.005;
  CRGB c = CRGB(0,0,0);
  int blend = 160;

  FastLED.clear();
  
  for (int i = 0; i<NUM_LEDS; i++){    
    // z anim  
    target = 140+cos(counter/700.0)*130;
    target = constrain(target, 0, 255);
    float dz = (zi - points[i].z);
    if (abs(dz) < target) {
        float off = constrain(target - abs(dz), 0, max_range);
        c = CRGB(0, 0, map(off, min_off, target, 0, 200));
        nblend(leds[i], c, blend);
        //leds[i] = c;
    }
    zi = (zi+speed*cos(counter/2000.0)*2);
    zi = constrain(zi, -max_range, max_range);
    if (abs(zi)==max_range) zi=-zi;

    // y anim
    float dy = (yi - points[i].y);
    if (abs(dy) < target) {
        float off = constrain(target - abs(dy), 0, max_range);
        c = CRGB(map(off, min_off, target, 0, 200), 0, 0);
        nblend(leds[i], c, blend);
        //leds[i] = c;
    }
    yi = (yi+speed*constrain(tan(counter/1600.0)/4, -3, 3));
    yi = constrain(yi, -max_range, max_range);
    if (abs(yi)==max_range) yi=-yi;
  
    // x anim
    float dx = (xi - points[i].x);
    if (abs(dx) < target) {
        float off = constrain(target - abs(dx), 0, max_range);
        c = CRGB(0, map(off, min_off, target, 0, 200), 0);
        nblend(leds[i], c, blend);
        //leds[i] = c;
    } 
    xi = (xi+speed*sin(counter/4000.0)*2);
    xi = constrain(xi, -max_range, max_range);
    if (abs(xi)==max_range) xi=-xi;
  }
  fadeToBlackBy(leds, NUM_LEDS, 35);  
  FastLED.show();
  counter++;
  delayMicroseconds(50);
}


#define NUM_PARTICLES 80
Particle *particles[NUM_PARTICLES];




void reset_particle(Particle *p){
  p->reset();
  p->led_number = (NUM_SIDES-1)*LEDS_PER_SIDE + random(11);
  uint8_t lev = random(10,50);
  p->color = (uint32_t)CRGB(lev, random(100,230), lev);
  p->a = random(TWO_PI*1000)/1000.0;
  p->c = PI;
  p->cv = -random(60,200)/1000.0;
  p->av = 0;
}

void wandering_particles(){
  for (int p=0; p<NUM_PARTICLES; p++){
    particles[p]->tick();
    // loop through path and light up LEDs a little bit with nblend
    int led = particles[p]->led_number;
    nblend(leds[led], particles[p]->color, 300/particles[p]->hold_time);
    if (random(2000) < 2){
      reset_particle(particles[p]);
    }
  }
  for (int i=0; i<NUM_LEDS; i++){
    leds[i].fadeToBlackBy(10+random(5));
  }
  FastLED.show();
  //delay(2);
}


void drip_particles(){
  // todo
}


CRGB bg_color;           // Current background color
CRGB line_color;         // Current line color
CRGB target_bg_color;    // Target background color
CRGB target_line_color;  // Target line color
bool dark_lines = true;  // Whether lines should be darker than background
bool in_transition = false;  // Add this line

// Function implementations (place these before pick_new_colors)
float get_perceived_brightness(const CRGB& color) {
    return (0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b) / 255.0;
}

float get_contrast_ratio(const CRGB& color1, const CRGB& color2) {
    float l1 = get_perceived_brightness(color1);
    float l2 = get_perceived_brightness(color2);
    float lighter = max(l1, l2);
    float darker = min(l1, l2);
    return (lighter + 0.05) / (darker + 0.05);
}

float get_hue_distance(const CRGB& color1, const CRGB& color2) {
    CHSV hsv1 = rgb2hsv_approximate(color1);
    CHSV hsv2 = rgb2hsv_approximate(color2);
    
    float diff = fabs(hsv1.h - hsv2.h);
    return min(diff, 255.0f - diff) * (180.0f/255.0f);  // convert to degrees (0-180)
}

void pick_new_colors() {
    // Store previous target as current
  bg_color = target_bg_color;
  line_color = target_line_color;

  uint8_t best_bg_index = 0;
  uint8_t best_line_index = 0;
  float best_score = 0;

  dark_lines = random8() > 128;
  // Serial.printf("\nPicking new colors (want dark lines: %d)\n", dark_lines);

  for (int i = 0; i < 4; i++) {
    uint8_t test_bg = random8();
    uint8_t test_line = random8();
    
    CRGB test_bg_color = ColorFromPalette(highlightPalette, test_bg);
    CRGB test_line_color = ColorFromPalette(basePalette, test_line);
    
    float contrast = get_contrast_ratio(test_bg_color, test_line_color);
    float hue_dist = get_hue_distance(test_bg_color, test_line_color);
    float bg_brightness = get_perceived_brightness(test_bg_color);
    float line_brightness = get_perceived_brightness(test_line_color);
    
    bool valid = dark_lines ? 
      (line_brightness < bg_brightness) : 
      (line_brightness > bg_brightness);
        
    float score = contrast * (hue_dist / 90.0f);
    
    // Serial.printf("Try %d: BG(%d,%d,%d) Line(%d,%d,%d) - ", i,
    //     test_bg_color.r, test_bg_color.g, test_bg_color.b,
    //     test_line_color.r, test_line_color.g, test_line_color.b);
    // Serial.printf("Contrast: %.2f HueDist: %.1f Score: %.2f Valid: %d\n",
    //     contrast, hue_dist, score, valid);
        
    if (valid && contrast >= 1.5 && hue_dist >= 30.0 && score > best_score) {
        best_score = score;
        best_bg_index = test_bg;
        best_line_index = test_line;
        // Serial.println("  ^ New best!");
    }
}

    target_bg_color = ColorFromPalette(highlightPalette, best_bg_index);
    target_line_color = ColorFromPalette(basePalette, best_line_index);
    
    // Convert to HSV for adjustments
    CHSV bg_hsv = rgb2hsv_approximate(target_bg_color);
    CHSV line_hsv = rgb2hsv_approximate(target_line_color);
    
    // Randomly adjust hue by up to +/- 16 steps (about 15 degrees)
    bg_hsv.h = bg_hsv.h + random8(33) - 16;
    line_hsv.h = line_hsv.h + random8(33) - 16;
    
    // Randomly adjust brightness
    if (dark_lines) {
        // Brighten background, darken lines
        bg_hsv.v = qadd8(bg_hsv.v, random8(64));
        line_hsv.v = qsub8(line_hsv.v, random8(64));
    } else {
        // Darken background, brighten lines
        bg_hsv.v = qsub8(bg_hsv.v, random8(64));
        line_hsv.v = qadd8(line_hsv.v, random8(64));
    }
    
    // Randomly adjust saturation a bit too
    bg_hsv.s = qadd8(qsub8(bg_hsv.s, random8(32)), random8(32));
    line_hsv.s = qadd8(qsub8(line_hsv.s, random8(32)), random8(32));
    
    target_bg_color = CHSV(bg_hsv.h, bg_hsv.s, bg_hsv.v);
    target_line_color = CHSV(line_hsv.h, line_hsv.s, line_hsv.v);
    
    Serial.printf("Final colors - BG: %d,%d,%d Line: %d,%d,%d Score: %.2f\n", 
        target_bg_color.r, target_bg_color.g, target_bg_color.b,
        target_line_color.r, target_line_color.g, target_line_color.b,
        best_score);
}

void blend_to_target(float blend_amount) {
  // Blend current colors toward target colors
  nblend(bg_color, target_bg_color, blend_amount * 255);
  nblend(line_color, target_line_color, blend_amount * 255);
}

// Helper function to find the smallest angle difference
float angle_diff(float a1, float a2) {
  float diff = fmod(abs(a1 - a2), TWO_PI);
  return min(diff, TWO_PI - diff);
}

void orientation_demo() {
  static uint16_t transition_counter = 0;
  static const uint16_t TRANSITION_START = 500;    
  static const uint16_t TRANSITION_DURATION = 250;
  static float anim_time = 1000.0;
  static float base_angle = 0.0;
  
  // Grid configuration
  static const int lat_lines = 6;
  static const int lon_lines = 6;
  static const float line_width = 0.11;
  
  // Check if it's time to start a new transition
  if (++transition_counter >= TRANSITION_START) {
    if (!in_transition) {
      pick_new_colors();
      in_transition = true;
      transition_counter = 0;
    }
  }
  
  // Calculate blend amount during transition
  float blend_amount = 0;
  float rotation_speed = 1.0;
  if (in_transition) {
    blend_amount = (float)transition_counter / TRANSITION_DURATION;
    if (blend_amount >= 1.0) {
      in_transition = false;
      blend_amount = 1.0;
    }
    rotation_speed = 0.3 + (0.7 * blend_amount * blend_amount);
    blend_to_target(blend_amount);
  }

  // Update rotation angles
  base_angle += 0.025 * rotation_speed;
  float spin = base_angle;
  float tilt = sin(anim_time * 0.002) * 0.3;
  float tumble = base_angle * 0.25;
  
  Matrix3d rot_z = AngleAxisd(spin, Vector3d::UnitZ()).matrix();
  Matrix3d rot_x = AngleAxisd(tilt, Vector3d::UnitX()).matrix();
  Matrix3d rot_y = AngleAxisd(tumble, Vector3d::UnitY()).matrix();
  Matrix3d rotation = rot_z * rot_x * rot_y;

  // Draw the grid
  for (int i = 0; i < NUM_LEDS; i++) {
    LED_Point p = points[i];
    Vector3d point(p.x, p.y, p.z);
    Vector3d rotated_point = rotation * point;
    Vector3d ray_dir = rotated_point.normalized();
    
    float a = atan2(ray_dir.y(), ray_dir.x());
    float c = acos(ray_dir.z());
    
    float a_norm = fmod(a + TWO_PI, TWO_PI);
    float c_norm = c;
    
    float lat_spacing = TWO_PI / lat_lines;
    float lon_spacing = PI / lon_lines;
    
    float nearest_lat = round(a_norm / lat_spacing) * lat_spacing;
    float nearest_lon = round(c_norm / lon_spacing) * lon_spacing;
    
    float lat_angle = min(
      abs(angle_diff(a_norm, nearest_lat)),
      abs(angle_diff(a_norm, nearest_lat + lat_spacing))
    );
    
    float lon_angle = min(
      abs(angle_diff(c_norm, nearest_lon)),
      abs(angle_diff(c_norm, nearest_lon + lon_spacing))
    );
    
    float point_dist = point.norm();
    float lat_dist = point_dist * lat_angle;
    float lon_dist = point_dist * lon_angle;
    
    float dist = min(lat_dist, lon_dist);
    float scaled_width = line_width * point_dist;
    
    if (dist < scaled_width) {
      uint8_t line_intensity = 255 * (1.0 - dist/scaled_width);
      CRGB line = line_color;
      line.nscale8(scale8(line_intensity, BRIGHTNESS));
      nblend(leds[i], line, line_intensity/2);
    } else {
      leds[i] = bg_color;
      leds[i].nscale8(scale8(180, BRIGHTNESS));  // Increased from 40 to 180 for more visible background
    }
  }
  
  anim_time += 1.0;
  FastLED.show();
}



void identify_sides(){
  // identify each side uniquely
  for (int s=0; s<NUM_SIDES; s++){
    CRGB side_color = CHSV(s*255/NUM_SIDES, 255, 255); // new color for each side
    // light N leds in the center, where N=side number
    for (int i=0; i<=s; i++){
      leds[s*LEDS_PER_SIDE+i] = side_color;
    }
    // light up the top row of LEDs
    for (int i=62; i<71; i++){
      if (i==63 or i==69) continue;  // skip corners due to ordering
      leds[s*LEDS_PER_SIDE+i] = side_color;
    }
  }
  fadeToBlackBy(leds, NUM_LEDS, 2);
  FastLED.show();
}

uint32_t FreeMem(){ // for Teensy 3.0
    uint32_t stackTop;
    uint32_t heapTop;

    // current position of the stack.
    stackTop = (uint32_t) &stackTop;

    // current position of heap.
    void* hTop = malloc(1);
    heapTop = (uint32_t) hTop;
    free(hTop);

    // The difference is (approximately) the free, available ram.
    return stackTop - heapTop;
}


// Function to convert RGB to ANSI 24-bit color codes
void printAnsiColor(const CRGB& color) {
    // ANSI escape sequence for setting foreground and background to 24-bit color
    Serial.print("\033[38;2;"); // Start foreground color
    Serial.print(color.r); Serial.print(";");
    Serial.print(color.g); Serial.print(";");
    Serial.print(color.b); Serial.print("m");

    Serial.print("\033[48;2;"); // Start background color
    Serial.print(color.r); Serial.print(";");
    Serial.print(color.g); Serial.print(";");
    Serial.print(color.b); Serial.print("m");

    // Print a space to display the color
    Serial.print("  ");

    // Reset colors
    Serial.print("\033[0m");
}

void timerStatusMessage(){
  Serial.printf("--> %d FPS @ mode:%d <--\n", FastLED.getFPS(), mode);
  Serial.printf("Power: %d mw\n", calculate_unscaled_power_mW(leds, NUM_LEDS));
  if (mode==0){   // wandering blobs
    int blob_id = 1;
    printAnsiColor(blobs[blob_id]->color);
    Serial.printf(" (%s) id=%d\n", getClosestColorName(blobs[blob_id]->color), blobs[blob_id]->blob_id);
    Serial.printf("Blob age: %d/%d\n", blobs[blob_id]->age, blobs[blob_id]->lifespan);
    Serial.printf("Blob av/cv: %0.3f %0.3f\n", blobs[blob_id]->av, blobs[blob_id]->cv);
    Serial.printf("Blob a/c: %0.2f %0.2f\n", blobs[blob_id]->a, blobs[blob_id]->c);
    Serial.printf("Blob x/y/z: %d %d %d\n", blobs[blob_id]->x(), blobs[blob_id]->y(), blobs[blob_id]->z());
  }
  if (mode==2){  // fade cycle
    Serial.printf("color_mix: %d/%d ", color_mix * 100 / 256, (256 - color_mix) * 100 / 256);
    Serial.printf("power_fade: %d ", power_fade);
    Serial.printf("num_picks: %d\n", num_picks);
    printAnsiColor(c);
    Serial.printf(" color1: %02hhX%02hhX%02hhX (%s) blend1: %d%%\n", c.r, c.g, c.b, getClosestColorName(c), blend1 * 100 / 256);
    printAnsiColor(c2);
    Serial.printf(" color2: %02hhX%02hhX%02hhX (%s) blend2: %d%%\n", c2.r, c2.g, c2.b, getClosestColorName(c2), blend2 * 100 / 256);
//    Serial.printf("blend1: %d blend2: %d\n", blend1, blend2);
  }
  if (mode==3){   // color_show
    Serial.printf("show_pos: %d\n", show_pos);
    Serial.printf("show_color: %d\n", show_color);
  }
  if (mode==4){   // wandering_particles
    Serial.printf("active particles: %d\n", NUM_PARTICLES);

    //Serial.printf("pos: %f,%f,%f\n", particles[0]->x(), particles[0]->y(), particles[0]->z());
    //Serial.printf("a/c: %f,%f\n", particles[0]->a, particles[0]->c);
    //Serial.printf("av/cv: %f,%f\n", particles[0]->av, particles[0]->cv);
  }
  if (mode==5){   // geography show
    Serial.printf("spin_angle: %f\n", spin_angle);
    Serial.printf("shift: %f\n", shift);  
  }

  if (mode==6){   // noise
    Serial.printf("Analog noise pins: %d/%d\n", analogRead(ANALOG_PIN_A), analogRead(ANALOG_PIN_B));
  }

  if (mode==7){   // orientation demo
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;
    sox.getEvent(&accel, &gyro, &temp);
    
    // Display current colors
    Serial.printf("dark lines? %s\n", dark_lines ? "true" : "false");
    printAnsiColor(bg_color);
    Serial.printf(" Background (%s)\n", getClosestColorName(bg_color));
    printAnsiColor(line_color); 
    Serial.printf(" Lines (%s)\n", getClosestColorName(line_color));
    Serial.printf("Brightness: %d%%\n", (BRIGHTNESS * 100) / 255);
    
    if (in_transition) {
      Serial.printf("Transitioning to:\n");
      printAnsiColor(target_bg_color);
      Serial.printf(" Target bg (%s)\n", getClosestColorName(target_bg_color));
      printAnsiColor(target_line_color);
      Serial.printf(" Target lines (%s)\n", getClosestColorName(target_line_color));
    }

#ifdef USE_IMU
    // Display sensor data
    Serial.printf("LSM6DSOX:\n");
    Serial.printf("\tTemperature: %.2f deg C\n", temp.temperature);
    Serial.printf("\tAccel X: %.2f \tY: %.2f \tZ: %.2f m/s^2\n", 
      accel.acceleration.x, accel.acceleration.y, accel.acceleration.z);
    Serial.printf("\tGyro X: %.2f \tY: %.2f \tZ: %.2f radians/s\n", 
      gyro.gyro.x, gyro.gyro.y, gyro.gyro.z);
  }
#endif

}

void setup() {
  float temp = InternalTemperature.readTemperatureC();
  random_seed = (temp - int(temp)) * 100000; 
  random_seed += (analogRead(ANALOG_PIN_A) * analogRead(ANALOG_PIN_B));
  randomSeed(random_seed);

  seed1 = random(random_seed) * 2 % 4000;
  seed2 = random(random_seed) * 3 % 5000;
  Serial.begin(115200);
  delay(300);
  Serial.printf("Start: DodecaRGBv2 firmware v%s\n", VERSION);
  Serial.printf("Teensy version: %d\n", TEENSYDUINO);
  // Parse FastLED version
  int major = FASTLED_VERSION / 1000000;
  int minor = (FASTLED_VERSION / 1000) % 1000;
  int patch = FASTLED_VERSION % 1000;
  Serial.printf("FastLED version: %d.%d.%d\n", major, minor, patch);
  Serial.printf("Compiled: %s %s\n", __DATE__, __TIME__);
  Serial.printf("CPU Temp: %f c\n", temp);
  Serial.printf("Num LEDs: %d\n", NUM_LEDS);
  Serial.printf("Random seed: %d\n", random_seed);

#ifdef USE_IMU
  Serial.println("Adafruit LSM6DSOX check");
  if (!sox.begin_I2C(LSM6DS_I2CADDR_DEFAULT, &Wire1)) {
    Serial.println("LSM6DSOX not found!");
  } else {
    Serial.println("LSM6DSOX found");
  }
#endif

  Serial.println();

  pinMode(ON_BOARD_LED, OUTPUT);
  pinMode(ANALOG_PIN_A, INPUT);
  pinMode(ANALOG_PIN_B, INPUT);
  pinMode(USER_BUTTON, INPUT_PULLUP);

  // set up fastled - two strips on two pins
  // see https://github.com/FastLED/FastLED/wiki/Parallel-Output#parallel-output-on-the-teensy-4
  FastLED.addLeds<WS2812, LED_CHANNEL_1_PIN, GRB>(leds, NUM_LEDS/2);
  FastLED.addLeds<WS2812, LED_CHANNEL_2_PIN, GRB>(leds, NUM_LEDS/2, NUM_LEDS/2);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setDither(0);
  FastLED.setMaxRefreshRate(60);
  FastLED.clear();
  FastLED.show();

  for (int side=0; side<NUM_SIDES; side++){
    leds[side*LEDS_PER_SIDE] = ColorFromPalette(RainbowColors_p, side * 255 / NUM_SIDES);
  }
  FastLED.show();

  // init Points
  for (int p=0; p < NUM_LEDS; p++){
    int side = p/LEDS_PER_SIDE;
    int side_led = p%LEDS_PER_SIDE;

    if (side_led == 0){
      Serial.printf("Calculating points, side %d\n",side);
      // GC.Collect();
      // GC.WaitForPendingFinalizers();
    }

    points[p].find_nearest_leds();

    if (side_led > 5 && side_led < 16 ){
      CRGB side_color =  ColorFromPalette(RainbowColors_p, side * 255 / NUM_SIDES);
      leds[p] = side_color;
      FastLED.show();
    }

    if (side_led%LEDS_PER_SIDE == 0){
      Serial.printf(" led %d, side %d, free mem=%u kb\n", p, side, FreeMem()/1024);
    }

  }

  // init Blobs
  for (int b=0; b<NUM_BLOBS; b++){
    blobs[b] = new Blob(b);
    // use unique ID to assign colors from uniquePalette
    blobs[b]->color = ColorFromPalette(uniquePalette, b * 16);
  }
  // init Particles
  for (int p=0; p<NUM_PARTICLES; p++){
    particles[p] = new Particle();
    reset_particle(particles[p]);
  }

  Serial.println("Init done");

  FastLED.clear();
  FastLED.show();

  delay(300);

  // inital mode at startup
  mode = 0;

  // Add to setup() after FastLED initialization
  // Initialize orientation demo colors
  target_bg_color = ColorFromPalette(highlightPalette, random8());
  target_line_color = ColorFromPalette(basePalette, random8());
  bg_color = target_bg_color;
  line_color = target_line_color;
}

#define NUM_MODES 8
long interval, last_interval = 0;
const long max_interval = 3000;
void loop() {
  interval = millis()/max_interval;
  if (random8(100)==0){
    int led_fade_level = map((millis() % max_interval), 0, max_interval, -256, 256);    
    analogWrite(ON_BOARD_LED, abs(led_fade_level));
  }
  if (interval != last_interval){
    timerStatusMessage();
    last_interval = interval;
  }
  // handle button press for mode change
  if (digitalRead(USER_BUTTON) == LOW){
    Serial.print("Button pressed, changing mode to ");
    mode++;
    mode %= NUM_MODES;
    Serial.println(mode);
 
    while (digitalRead(USER_BUTTON) == LOW){
      CRGB c = CRGB::White;
      FastLED.setBrightness(BRIGHTNESS);
      c.setHSV(millis()/20 % 255, 255, 64);
      FastLED.showColor(c);
      FastLED.show(); 
      delay(20); 
    }
    Serial.println("Button released");
  }
  if (mode == 0){
    orbiting_blobs();
    //delay(1);
  }
  if (mode == 1){
    fade_test();
    // delay(2);
  }
  if (mode == 2){
    flash_fade_points();
    // delay(2);
  }
  if (mode==3){
    //solid_sides();
    color_show();
  }
  if (mode==4){
    wandering_particles();
    // delay(1);
    //drip_particles();
  }
  if (mode==5){
    geography_show();
    // delay(1);
  }
  if (mode==6){
    tv_static();
    // delay(1);
  }
  if (mode==7){
    orientation_demo();
  }
}
