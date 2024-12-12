#include <Arduino.h>
#include <cmath>
#include <FastLED.h>
// #include "Ticker.h"
#include "points.h"
//#include "network.h"
#include "blob.h"
#include "particle.h"

/*

DodecaRBG V2

NOTE (Dec 8 2024):
- refactoring in progress
- working, but only tested with 6 sides active (624) and frame rates of 50fps are achievable so far

We have a dodecahedron model with 12 sides. 
Each side is a pentgon-shaped PCB circuit board with RGB LEDs arranged on them, spaced evenly. 

A Teensy 3.6+ microcontroller is used to control the LEDs, using the Arduino environment and FastLED library.

Each pentagon side contains 104 RGB leds on a circuit board, arranged in rings from the center.

Each side connects to the next, in series, for a grand total of 1247 LEDs. 

As the PCB circuit boards are wired together to form the dodecahedron, the arrangement of the sides must be 
defined, as there are many possible configurations. 
In addition, the rotation of each side must be defined, as it can have five possible rotations.

There's a processing sketch at https://github.com/somebox/dodeca-rgb-simulator that generates the list
of points, the X,Y,Z coordinates, and defines the order of the sides and their rotations. It also
renders an interactive 3D model of the dodecahedron.

*/

// LED configs
#define USER_BUTTON 2
// https://github.com/FastLED/FastLED/wiki/Parallel-Output#parallel-output-on-the-teensy-4
// pins 19+18 are used to control two strips of 624 LEDs, for a total of 1248 LEDs
#define WS2812_LED_PIN 19  // Teensy 4.1/fastled pin order: .. ,19,18,14,15,17, ..
#define ANALOG_PIN_A 24
#define ANALOG_PIN_B 25
#define ON_BOARD_LED 13

#define BRIGHTNESS  40
#define WIFI_ENABLED false

// base color palette
#define NUM_COLORS 11
CRGB my_colors[] = {
  CHSV(60, 255, 128),    // Bright yellow
  CHSV(32, 200, 110),    // Radiant orange
  CHSV(160, 179, 100),   // Sky blue
  CHSV(96, 160, 70),     // Forest green
  CHSV(213, 250, 80),    // Rich purple
  CHSV(240, 80, 90),    // Soft pink
  CHSV(125, 190, 90),    // Deep aqua
  CHSV(64, 50, 90),     // Creamy white
  CHSV(31, 140, 90),    // Warm brown
  CHSV(180, 10, 65),       // Cool gray
  CRGB::DarkMagenta,
  CRGB::DarkBlue
};

bool led_toggle = false;

CRGB leds[NUM_LEDS];

// Constants
const int LEDS_PER_RING = 10;
const int LEDS_PER_EDGE = 15;

int mode = 0;

void color_show(){
  // turn off all LEDs in a dissolving pattern
  for (int x=0; x<100; x++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].fadeToBlackBy(1+random(3));
    }
    if (digitalRead(USER_BUTTON) == LOW){
      return;
    } else {
      FastLED.show();
      delay(3);
    }
  }
  
  // light up all LEDs in sequence
  CRGB color = CRGB::White;
  int c = random(1,255);
  for (int i = 0; i < NUM_LEDS; i++) {
    color.setHSV((c+i/2)%255, 255, 128);
    leds[i] = color;
    if (digitalRead(USER_BUTTON) == LOW){
      return;
    } else {
      delay(3);
      FastLED.show();
    }
  }
  delay(500);

}

void solid_sides(){
  static int s=0;
  if (random(12)==0) { s = random(NUM_SIDES); };
  CRGB c = CHSV(millis()/random(100,1000) % 255, random(150)+100, random(120)+10);
  for (int level=0; level<50; level+=1){
    for (int i=s*LEDS_PER_SIDE; i<(s+1)*LEDS_PER_SIDE; i++){
      nblend(leds[i], c, 3);
    }
    FastLED.show();
    if (digitalRead(USER_BUTTON) == LOW) return;
    delayMicroseconds(100);
  } 
  for (int level=100; level>0; level--){
    for (int i=(s)*LEDS_PER_SIDE; i<(s+1)*LEDS_PER_SIDE; i++){
      nblend(leds[i], CHSV(millis()/random(100,1000) % 255, random(150)+100, random(120)+10), 3);
    }
    FastLED.show();
    if (digitalRead(USER_BUTTON) == LOW) return;
    delayMicroseconds(100);
  }
  FastLED.show();
  s = (s+1) % NUM_SIDES;
}

// randomly light up LEDs and fade them out individually, like raindrops
uint8_t fade_level = 20;
uint8_t fade_level2 = 10;
int chance1=30;
int chance2=50;
void flash_fade_points(){
  // cycle color over time
  CRGB c = CRGB(
    sin8_C((millis()/330)%255),
    sin8_C((millis()/220)%100+150),
    sin8_C((millis()/150)%155+50)
  ); 
  CRGB c2 = CRGB(
    sin8_C((millis()/420)%255),
    sin8_C((millis()/130)%50+200),
    sin8_C((millis()/350)%150+50)
  ); 
  fade_level = sin8_C((millis()/600)%255);
  fade_level2 = sin8_C((millis()/810)%255);
  for (int n=0; n<NUM_SIDES; n++){
    if (random(sin8_C((millis()/340)%255)) < chance1) {
      int r1 = random(n*LEDS_PER_SIDE, (n+1)*LEDS_PER_SIDE);        
      nblend(leds[r1], c, map(fade_level, 0, 255, 20, 50));
    }
    if (random(sin8_C((millis()/570)%255)) < chance2) {
      int r2 = random(n*LEDS_PER_SIDE, (n+1)*LEDS_PER_SIDE);
      nblend(leds[r2], c2, map(fade_level2, 0, 255, 20, 50));        
    }
  }
  // for (int i = 0; i < NUM_LEDS; i++){
  //   if (random(255) < fade_level/2) leds[i].fadeToBlackBy(map(fade_level, 0, 255, 50, 15));
  // }

  chance1 = map(sin(millis()/7000.0), -1.0, 1.0, 0, 130);
  chance2 = map(cos(millis()/19000.0), -1.0, 1.0, 0, 140);
  fadeToBlackBy(leds, NUM_LEDS, map((fade_level+fade_level2)/2, 0, 265, 6, 0));
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
    leds[i] = CHSV(analogRead(ANALOG_PIN_A), 255, analogRead(ANALOG_PIN_B)/2);
  }
  FastLED.show();
}

float spin_angle = 0;
float shift = 0;
void geography_show(){
  static int sphere_r = 310; // radius of sphere

  // Lorenz parameters
  static float sigma = 8.0+random(400)/100.0;  // Prandtl number
  static float rho = 24.0+random(400)/100.0;    // Rayleigh number
  static float beta = 8.0 / 3+random(100)/100.0;

  // Time step
  static float dt = 0.001;

  // State variables
  static float x = 0.1, y = 0.0, z = 0.0;  // Initial conditions
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
  float normalized_x = (x + 20.0) / 40.0 * 2.0 - 0.8;  // Assuming x typically stays within [-20, 20]
  float normalized_y = (y + 30.0) / 40.0 * 2.0 - 0.4;  // Assuming x typically stays within [-20, 20]
  float normalized_z = (z + 10.0) / 20.0 * 2.0 - 0.9;  // Assuming x typically stays within [-20, 20]

  for (int i = 0; i<NUM_LEDS; i++){
    float a = acos(points[i].y / sphere_r)+sin(normalized_y);
    float c = atan2(points[i].z, points[i].x)+cos(spin_angle)+sin(normalized_x);
    int c_start = map(normalized_y+a, -2.5, 3.0, 0, 255);
    int c_end = map(normalized_x+c, -2.5, 3.0, 0, 255);
    int hue = map(fmod(normalized_x/10.0+normalized_y/10.0+c, PI), 0, PI, c_start, c_end);
    int brightness = map(sin(a*shift+cos(spin_angle)), -3.3, 3.0, 5, 130);
    leds[i] = CHSV(hue, 255, brightness);
  }
  FastLED.show();
  spin_angle += 0.01+normalized_z/100.0;
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
  
  // // the point on the sphhere
  // float x1 = r * sin(c)*cos(a);
  // float y1 = r * sin(c)*sin(a);
  // float z1 = r * cos(c);
  
  for (int b=0; b<NUM_BLOBS; b++){
    auto rad_sq = blobs[b]->radius*blobs[b]->radius;
    for (int i = 0; i<NUM_LEDS; i++){ 
      int dist = calculateBlobDistance(points[i], blobs[b]);
      if (dist < rad_sq){
        CRGB c = blobs[b]->color;
        // slow fade in
        if (blobs[b]->age < 150){
          c.fadeToBlackBy(map(blobs[b]->age, 0, 150, 230, 2));
        }
        nblend(leds[i], c, map(dist, 0, rad_sq, 30, 5));
      }
    }
  }

  fadeToBlackBy(leds, NUM_LEDS, 5);  
  // for (int i=0; i<NUM_LEDS; i++){
  //   leds[i].fadeToBlackBy(10);
  // }

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
  float speed = 0.01;
  CRGB c = CRGB(0,0,0);
  int blend = 128;

  FastLED.clear();
  
  for (int i = 0; i<NUM_LEDS; i++){    
    // z anim  
    float dz = (zi - points[i].z);
    target = 140+sin(counter/700.0)*130;
    target = constrain(target, 20, 260);
    if (abs(dz) < target) {
        float off = target - abs(dz);
        c = CRGB(0, 0, map(off, 0, target, 0, 255));
        nblend(leds[i], c, blend);
    }
    zi = (zi+speed*cos(counter/2000.0)*2);
    zi = constrain(zi, -max_range, max_range);
    if (abs(zi)==max_range) zi=-zi;

    // y anim
    float dy = (yi - points[i].y);
    if (abs(dy) < target) {
        float off = target - abs(dy);
        c = CRGB(map(off, 0, target, 0, 255), 0, 0);
        nblend(leds[i], c, blend);
    }
    yi = (yi+speed*constrain(tan(counter/1600.0)/4, -3, 3));
    yi = constrain(yi, -max_range, max_range);
    if (abs(yi)==max_range) yi=-yi;
  
    // x anim
    float dx = (xi - points[i].x);
    if (abs(dx) < target) {
        float off = target - abs(dx);
        c = CRGB(0, map(off, 0, target, 0, 255), 0);
        nblend(leds[i], c, blend);
    } 
    xi = (xi+speed*sin(counter/4000.0)*2);
    xi = constrain(xi, -max_range, max_range);
    if (abs(xi)==max_range) xi=-xi;
  }
  FastLED.show();
  counter++;
}


#define NUM_PARTICLES 25
Particle *particles[NUM_PARTICLES];

void timerStatusMessage(){
  Serial.printf("FPS: %d\n", FastLED.getFPS());
  if (mode==0){
    Serial.printf("Blob age: %d/%d\n", blobs[0]->age, blobs[0]->lifespan);
    Serial.printf("Blob av/cv: %d %d\n", blobs[0]->av, blobs[0]->cv);
    Serial.printf("Blob a/c: %d %d\n", blobs[0]->a, blobs[0]->c);
    Serial.printf("Blob x/y/z: %d %d %d\n", blobs[0]->x(), blobs[0]->y(), blobs[0]->z());
  }
  if (mode==2){
    Serial.printf("fade_level: %d\n", fade_level);
    Serial.printf("fade_level2: %d\n", fade_level2);
    Serial.printf("chance1: %d chance2: %d\n", chance1, chance2);
  }
  if (mode==4){
    // wandering particles
    Serial.printf("pos: %f,%f,%f\n", particles[0]->x(), particles[0]->y(), particles[0]->z());
    Serial.printf("a/c: %f,%f\n", particles[0]->a, particles[0]->c);
    Serial.printf("av/cv: %f,%f\n", particles[0]->av, particles[0]->cv);
  }
  if (mode==5){
    Serial.printf("spin_angle: %f\n", spin_angle);
    Serial.printf("shift: %f\n", shift);  
    Serial.printf("Analog noise pins: %d/%d\n", analogRead(ANALOG_PIN_A), analogRead(ANALOG_PIN_B));
  }
  // toggle on-board LED
  // digitalWrite(ON_BOARD_LED, led_toggle);
  led_toggle = !led_toggle;
}
//Ticker timer1;


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
    leds[i].fadeToBlackBy(5+random(10));
  }
  FastLED.show();
  //delay(2);
}


void drip_particles(){
  // todo
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


void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("Start");

  pinMode(ON_BOARD_LED, OUTPUT);
  pinMode(ANALOG_PIN_A, INPUT);
  pinMode(ANALOG_PIN_B, INPUT);
  pinMode(USER_BUTTON, INPUT_PULLUP);

  // set up fastled - two strips on two pins
  // see https://github.com/FastLED/FastLED/wiki/Parallel-Output#parallel-output-on-the-teensy-4
  FastLED.addLeds<2, WS2812, WS2812_LED_PIN, GRB>(leds, NUM_LEDS/2);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setDither(1);
  FastLED.setMaxRefreshRate(60);
  FastLED.clear();
  FastLED.show();

  for (int side=0; side<NUM_SIDES; side++){
    leds[side*LEDS_PER_SIDE] = my_colors[side];
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

    Serial.printf(" led %d, side %d, free mem=%u kb\n", p, side, FreeMem()/1024);

    if (side_led > 5 && side_led < 16 ){
      leds[p] = my_colors[side];
      FastLED.show();
    }
  }

  // init Blobs
  for (int b=0; b<NUM_BLOBS; b++){
    blobs[b] = new Blob();
    blobs[b]->color = CHSV(random(255), random(150)+100, random(120)+100);
  }
  // init Particles
  for (int p=0; p<NUM_PARTICLES; p++){
    particles[p] = new Particle();
    reset_particle(particles[p]);
  }

  Serial.println("Init done");

  // for (int i=1; i<LEDS_PER_SIDE; i+=(1+random(4))){    
  //   for (int side=0; side<NUM_SIDES; side++){
  //     leds[side*LEDS_PER_SIDE+i] = my_colors[side];
  //   }
  //   FastLED.show();
  //   delay(50);
  // }

  if (WIFI_ENABLED == true){
    // bool config_wifi = (digitalRead(USER_BUTTON) == LOW);
    // bool connected = ConnectToWifi(config_wifi);

    // // flash green if connected, or red if not
    // for (int x=0; x<40; x++){
    //   int level = 150 - abs(map(x, 0,  40, -150, 150));
    //   CRGB c =  (connected ? CRGB(0,level,0) : CRGB(level,0,0));
    //   FastLED.showColor(c);
    //   FastLED.show();
    //   delayMicroseconds(500);
    // }
  } else {
    Serial.println("Wifi disabled.");
  }

  FastLED.clear();
  FastLED.show();

  delay(300);

//  timer1.attach(3, timerStatusMessage);

  mode = 0;

}


long interval, last_interval = 0;
const long max_interval = 3000;
void loop() {
  interval = millis()/max_interval;
  int led_fade_level = map((millis() % max_interval), 0, max_interval, -256, 256);
  analogWrite(ON_BOARD_LED, abs(led_fade_level));
  if (interval != last_interval){
    timerStatusMessage();
    last_interval = interval;
  }
  // handle button press for mode change
  if (digitalRead(USER_BUTTON) == LOW){
    Serial.print("Button pressed, changing mode to ");
    mode = (mode + 1) % 7;
    Serial.println(mode);
 
    while (digitalRead(USER_BUTTON) == LOW){
      CRGB c = CRGB::White;
      FastLED.setBrightness(BRIGHTNESS);
      c.setHSV(millis()/20 % 255, 255, 128);
      FastLED.showColor(c);
      FastLED.show(); 
      delay(10); 
    }
    Serial.println("Button released");
  }
  if (mode == 0){
    orbiting_blobs();
    //delay(1);
  }
  if (mode == 1){
    fade_test();
    delay(2);
  }
  if (mode == 2){
    flash_fade_points();
    delay(2);
  }
  if (mode==3){
    //solid_sides();
    color_show();
  }
  if (mode==4){
    wandering_particles();
    // delay(1);
    // drip_particles();
  }
  if (mode==5){
    geography_show();
    // delay(1);
  }
  if (mode==6){
    tv_static();
    // delay(1);
  }
}