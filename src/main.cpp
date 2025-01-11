#include <Arduino.h>
#include <Wire.h>
#include <InternalTemperature.h>
#include <cmath>
#define FASTLED_USES_OBJECTFLED
#include <FastLED.h>

#include "points.h"
#include "particle.h"
#include "palettes.h"
#include "animation_manager.h"

#include <Adafruit_LSM6DSOX.h>

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

v2.5 Jan 12 2025:
- added wandering particles animation
- added xyz-scanner animation

v2.4 Jan 11 2025:
- refactoring: as the main file is getting too big, I'm moving the animation code to separate files
- add animation manager to handle animations, and animation params to handle parameters
- refactored palettes and color lookup to be included in the header file
- new logging system for animations
- add sparkles animation
- TODO: migrate more animations to new structure

v2.3 Jan 10 2025:
- update FastLED to 3.9.10
- improved orientation demo, colors and transitions

v2.2 Jan 1 2025:
- added orientation demo, with rotating sphere animations

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
#define VERSION "2.5.1"
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

#define BRIGHTNESS  50      // global brightness, should be used by all animations
#define USE_IMU true        // enable orientation sensor (currently: LSM6DSOX)




CRGB leds[NUM_LEDS];
AnimationManager animation_manager(leds, NUM_LEDS, NUM_SIDES);
Adafruit_LSM6DSOX sox;

// Constants
const int LEDS_PER_RING = 10;
const int LEDS_PER_EDGE = 15;

long random_seed = 0;
int seed1,seed2 = 0;
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
CRGB bg_color_prev = CRGB::Black ;
CRGB line_color_prev = CRGB::Black;
bool dark_lines = true;  // Whether lines should be darker than background
bool in_transition = true;  // active animation between colors and lines
const uint16_t cycle_time = 1500;    // time between color changes
const uint16_t transition_duration = 200;   // time to transition between colors

// Color selection, happens every transition
void pick_new_colors() {
    // Store previous target as current
    bg_color_prev = bg_color;
    line_color_prev = line_color;
    bg_color = target_bg_color;
    line_color = target_line_color;
    
    CHSV best_hsv1, best_hsv2;
    float best_score = 0;
    
    // find an acceptable random color pair
    for (int i = 0; i < 16; i++) {
        // Pick random colors from palettes and convert to HSV immediately
        CHSV hsv1 = rgb2hsv_approximate(ColorFromPalette(uniquePalette, random8()));
        CHSV hsv2 = rgb2hsv_approximate(ColorFromPalette(RainbowStripesColors_p, random8()));
        
        // Determine which color is brighter,         
        // apply random adjustments to relative brightness
        if (get_perceived_brightness(hsv1) > get_perceived_brightness(hsv2)) {
            hsv1.v = qadd8(hsv1.v, 16+random8(32));  // Make brighter color more bright
            hsv2.v = qsub8(hsv2.v, 16+random8(32));  // Make darker color more dark
        } else {
            hsv2.v = qadd8(hsv2.v, 16+random8(32));  // Make brighter color more bright
            hsv1.v = qsub8(hsv1.v, 16+random8(32));  // Make darker color more dark
        }
        
        float contrast = get_contrast_ratio(hsv1, hsv2);
        float hue_dist = get_hue_distance(hsv1, hsv2);
        
        // Score based on contrast and hue distance
        float score = contrast * (hue_dist / 180.0f);
        
        // Check if colors meet our criteria and have better score
        if (contrast >= 1.5 && hue_dist >= 20.0 && score > best_score) {
            best_score = score;
            best_hsv1 = hsv1;
            best_hsv2 = hsv2;
        }
    }

    // Fallback if no good pairs were found
    if (get_contrast_ratio(best_hsv1, best_hsv2) < 1.5) {
        Serial.println("Warning: No good color pairs found, using fallback colors");
        best_hsv1 = CHSV(random8(), 0, 200);
        best_hsv2 = CHSV(0, 0, 0);
    }

    // Convert to RGB for final colors
    CRGB bright_color = CHSV(best_hsv1.h, best_hsv1.s, best_hsv1.v);
    CRGB dark_color = CHSV(best_hsv2.h, best_hsv2.s, best_hsv2.v);
    bright_color.fadeLightBy(20);
    dark_color.fadeToBlackBy(50);

    // Randomly decide if lines should be darker than background
    dark_lines = !dark_lines;
    
    // Assign colors based on dark_lines setting
    if (dark_lines) {
        target_bg_color = bright_color;
        target_line_color = dark_color;
    } else {
        target_bg_color = dark_color;
        target_line_color = bright_color;
    }
    
    Serial.printf("New colors picked (dark_lines: %d) - Score: %.2f\n", 
        dark_lines, best_score);
}

void blend_to_target(float blend_amount) {
  // Blend current colors toward target colors
  bg_color = blend(bg_color_prev, target_bg_color, blend_amount * 255);
  line_color = blend(line_color_prev, target_line_color, blend_amount * 255);
}

// Helper function to find the smallest angle difference
float angle_diff(float a1, float a2) {
  float diff = fmod(abs(a1 - a2), TWO_PI);
  return min(diff, TWO_PI - diff);
}

void orientation_demo() {
  static uint16_t transition_counter = 900;
  static float tilt_speed = 0.0;
  static float rotation_angle = 0.0;
  
  // Grid configuration
  static const int lat_lines = 5;
  static const int lon_lines = 4;
  static float current_line_width = 0.14;
  static float target_line_width = 0.14;
  
  // Check if it's time to start a new transition
  if (++transition_counter >= cycle_time) {
    if (!in_transition) {
      pick_new_colors();
      in_transition = true;
      transition_counter = 0;
    }
  }
  
  // Calculate blend amount during transition
  float rotation_speed = 0.7;
  if (in_transition) {
    if (transition_counter == 0) {
      // Pick new target width at start of transition
      target_line_width = random(10, 40)/100.0;
    }
    // Smoothly blend between current and target width
    float blend_amount = map(transition_counter, 0, transition_duration, 0, 1000)/1000.0;
    current_line_width += (target_line_width - current_line_width) * blend_amount;
    // Smoothly blend colors
    blend_to_target(blend_amount);
    rotation_speed = 0.7 + sin(blend_amount*PI)*1.4;
    
    // End transition when complete
    if (transition_counter >= transition_duration) {
      in_transition = false;
    }
  }

  // Update rotation angles
  rotation_angle += 0.025 * rotation_speed;
  float spin = rotation_angle;
  float tilt = sin(tilt_speed * 0.002) * 0.5;
  float tumble = rotation_angle * 0.25;
  
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
    float scaled_width = current_line_width * point_dist;

    // set background color
    leds[i] = bg_color;
    leds[i].nscale8(scale8(160, BRIGHTNESS));  // scale down the brightness      

    // handle the transition between background and line
    if (dist < scaled_width) {
      // Create smoother falloff using cubic or quadratic easing
      float blend_factor = 1.0 - (dist/scaled_width);
      blend_factor = blend_factor * blend_factor * (3 - 2 * blend_factor); // Smoothstep function

      // set line color
      uint8_t line_intensity = 255 * blend_factor;
      CRGB line = line_color;
      line.nscale8(scale8(line_intensity, BRIGHTNESS));
      leds[i] = blend(leds[i], line, line_intensity);  // More subtle blending for high contrast
    }
  }
  
  tilt_speed += 1.0;
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


void timerStatusMessage(){
  Serial.printf("--> %d FPS @ mode:%d <--\n", FastLED.getFPS(), mode);
  Serial.printf("Power: %d mw\n", calculate_unscaled_power_mW(leds, NUM_LEDS));
  if (mode==0){   // wandering blobs
    String status = animation_manager.getCurrentAnimation()->getStatus();
    Serial.printf("Animation status: %s\n", status.c_str());
  }
  if (mode==1){  // fade test(): xyz intersection planes with fading lines
    String status = animation_manager.getCurrentAnimation()->getStatus();
    Serial.printf("Animation status: %s\n", status.c_str());
  }
  if (mode==2){  // Sparkles
    String status = animation_manager.getCurrentAnimation()->getStatus();
    Serial.printf("Animation status: %s\n", status.c_str());
    // Serial.printf("color_mix: %d/%d ", color_mix * 100 / 256, (256 - color_mix) * 100 / 256);
    // Serial.printf("power_fade: %d ", power_fade);
    // Serial.printf("num_picks: %d\n", num_picks);
  }
  if (mode==3){   // color_show
    Serial.printf("show_pos: %d\n", show_pos);
    Serial.printf("show_color: %d\n", show_color);
  }
  if (mode==4){   // wandering_particles
    String status = animation_manager.getCurrentAnimation()->getStatus();
    Serial.printf("Animation status: %s\n", status.c_str());

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
    Serial.print(getAnsiColorString(bg_color));
    Serial.printf(" Background (%s)\n", getClosestColorName(bg_color));
    Serial.print(getAnsiColorString(line_color)); 
    Serial.printf(" Lines (%s)\n", getClosestColorName(line_color));
    Serial.printf("Brightness: %d%%\n", (BRIGHTNESS * 100) / 255);
    
    if (in_transition) {
      Serial.printf("Transitioning to:\n");
      Serial.print(getAnsiColorString(target_bg_color));
      Serial.printf(" Target bg (%s)\n", getClosestColorName(target_bg_color));
      Serial.print(getAnsiColorString(target_line_color));
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
  random16_set_seed(random_seed);
  
  seed1 = random(random_seed) * 2 % 4000;
  seed2 = random(random_seed) * 3 % 5000;
  random16_add_entropy(seed1);
  random16_add_entropy(seed2);

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
  FastLED.setMaxRefreshRate(90);
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
    }

    points[p].find_nearest_leds();

    if (side_led > 5 && side_led < 16 ){
      CRGB side_color =  ColorFromPalette(RainbowColors_p, side * 255 / NUM_SIDES);
      leds[p] = side_color;
      FastLED.show();
    }
  }

  // init Particles
  for (int p=0; p<NUM_PARTICLES; p++){
    particles[p] = new Particle();
    reset_particle(particles[p]);
  }
  // init colors
  pick_new_colors();

  Serial.println("Init done");

  FastLED.clear();
  FastLED.show();

  delay(300);

  // Add to setup() after FastLED initialization
  // Initialize orientation demo colors
  target_bg_color = ColorFromPalette(highlightPalette, random8());
  target_line_color = ColorFromPalette(basePalette, random8());
  bg_color = target_bg_color;
  line_color = target_line_color;

  // Set up animations

  // Add animations with default settings
  animation_manager.add("blobs");
  animation_manager.add("xyz_scanner");  
  animation_manager.add("sparkles");
  animation_manager.add("wandering_particles");
  
  // Configure with presets
  animation_manager.preset("sparkles", "default");
  animation_manager.preset("xyz_scanner", "fast");  // Try different speeds
  animation_manager.preset("blobs", "fast");

  // inital mode at startup
  mode = 4; animation_manager.setCurrentAnimation(3);
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
 
    switch(mode) {
      case 0: animation_manager.setCurrentAnimation(0); break;  // blobs
      case 1: animation_manager.setCurrentAnimation(1); break;  // xyz_scanner
      case 2: animation_manager.setCurrentAnimation(2); break;  // sparkles
      case 4: animation_manager.setCurrentAnimation(3); break;  // wandering_particles
    }

    while (digitalRead(USER_BUTTON) == LOW){
      CRGB c = CRGB::White;
      FastLED.setBrightness(BRIGHTNESS);
      c.setHSV(millis()/100 % 255, 255, 64);
      FastLED.showColor(c);
      FastLED.show(); 
      delay(20); 
    }
    Serial.println("Button released");
  }
  if (mode == 0){
    animation_manager.update();
    FastLED.show();
  }
  if (mode == 1){
    animation_manager.update();
    FastLED.show();
  }
  if (mode == 2){
    animation_manager.update();
    FastLED.show();
  }
  if (mode==3){
    //solid_sides();
    color_show();
  }
  if (mode==4){
    animation_manager.update();
    FastLED.show();
  }
  if (mode==5){
    geography_show();
  }
  if (mode==6){
    tv_static();
  }
  if (mode==7){
    orientation_demo();
  }
}
