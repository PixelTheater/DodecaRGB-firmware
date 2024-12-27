#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <FastLED.h>
#include <cmath>
//#include "network.h"
// #include "Ticker.h"
#include "blob.h"
#include "points.h"
#include "particle.h"
#include <InternalTemperature.h>
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

// Teensy I2C on pins 17/16 (SDA,SCL) is mapped to Wire1 in Arduinio framework
// https://www.pjrc.com/teensy/td_libs_Wire.html
double xPos = 0, yPos = 0, headingVel = 0;
uint16_t BNO055_SAMPLERATE_DELAY_MS = 10; //how often to read data from the board
uint16_t PRINT_DELAY_MS = 500; // how often to print the data
uint16_t printCount = 0; //counter to avoid printing every 10MS sample

//velocity = accel*dt (dt in seconds)
//position = 0.5*accel*dt^2
double ACCEL_VEL_TRANSITION =  (double)(BNO055_SAMPLERATE_DELAY_MS) / 1000.0;
double ACCEL_POS_TRANSITION = 0.5 * ACCEL_VEL_TRANSITION * ACCEL_VEL_TRANSITION;
double DEG_2_RAD = 0.01745329251; //trig functions require radians, BNO055 outputs degrees

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29, &Wire1);


#define BRIGHTNESS  40
#define WIFI_ENABLED false

// base color palette
#define NUM_COLORS 16
DEFINE_GRADIENT_PALETTE(my_colors_gp) {
  0,   255, 255, 0,    // Bright yellow
  16,  255, 165, 0,    // Radiant orange
  32,  0,   255, 255,  // Sky blue
  48,  0,   128, 0,    // Forest green
  64,  128, 0,   128,  // Rich purple
  80,  255, 192, 203,  // Soft pink
  96,  0,   255, 255,  // Deep aqua
  112, 255, 255, 224,  // Creamy white
  128, 139, 69,  19,   // Warm brown
  144, 169, 169, 169,  // Cool gray
  160, 139, 0,   139,  // DarkMagenta
  176, 0,   0,   139,  // DarkBlue
  192, 255, 0,   0,    // Red
  208, 0,   255, 0,    // Green
  224, 0,   0,   255,  // Blue
  240, 255, 255, 255   // White
};

CRGBPalette16 my_colors = my_colors_gp;

CRGB leds[NUM_LEDS];

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
static int period = 520;
int seed1,seed2;
uint8_t blend1;
uint8_t blend2;
int power_fade=1;
void flash_fade_points(){
  color_mix = (int)(64 + sin8_C(millis()/(period*3.2)+seed1) / 2);
  // cycle color over time
  c = ColorFromPalette( PartyColors_p, sin8_C(millis()/(period*4.3)+seed1));
  c2 = ColorFromPalette( RainbowStripeColors_p, sin8_C(millis()/(period*5.1)+seed2));
  blend1=sin8_C(millis()/(period*4.2)+seed1);
  blend2=sin8_C(millis()/(period*3.5)+seed2);
  for (int n=0; n<NUM_SIDES; n++){
    int num_picks = (350-(blend1+blend2)/2)/10;  // as the blend decreases, the number of LEDs increases
    for (int m=0; m<num_picks; m++){
      if (random8(128) < color_mix) {
        int r1 = random(n*LEDS_PER_SIDE, (n+1)*LEDS_PER_SIDE);        
        nblend(leds[r1], c, map(blend1, 0, 255, 3, 30));
      }
      if (random8(128) < (256-color_mix)) {
        int r2 = random(n*LEDS_PER_SIDE, (n+1)*LEDS_PER_SIDE);
        nblend(leds[r2], c2, map(blend2, 0, 255, 3, 30));        
      }
    }
  }
  // fades
  int power = calculate_unscaled_power_mW(leds, NUM_LEDS);
  power_fade = (power_fade * 19 + max(map(power,6000,25000,3,50),1))/20;  
  for (int i = 0; i < NUM_LEDS; i++){
    //int v = leds[i].getAverageLight();
    leds[i].fadeToBlackBy(power_fade);
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
    leds[i] = CHSV(analogRead(ANALOG_PIN_A)/2, 255, analogRead(ANALOG_PIN_B)/5);
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

#define NUM_BLOBS 8
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
          c.fadeToBlackBy(map(blobs[b]->age, 0, 150, 180, 1));
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

void orientation_demo(){
  static int sphere_r = 310; // radius of sphere
  // static int last_sensor_read = 0; // tracks sensor readings as to not overload it
  sensors_event_t event;

  bno.getEvent(&event);
  
  float a2 = acos((float)event.orientation.y / sphere_r);
  float c2 = atan2((float)event.orientation.z, (float)event.orientation.x);
  for (int i=0; i<NUM_LEDS; i++){
    float a = acos(points[i].y / sphere_r);
    float c = atan2(points[i].z, points[i].x);
    float dist = sqrt((a - a2) * (a - a2) + (c - c2) * (c - c2)); // Corrected distance calculation
    CRGB color = CHSV(180, 255, map(dist, 0, PI, 150, 0)); // Adjusted brightness mapping
    leds[i] = color;
  }
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

void displaySensorDetails(void){
  sensor_t sensor;
  bno.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" xxx");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" xxx");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" xxx");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

void timerStatusMessage(){
  Serial.printf("--> %d FPS @ mode:%d <--\n", FastLED.getFPS(), mode);
  Serial.printf("Power: %d mw\n", calculate_unscaled_power_mW(leds, NUM_LEDS));
  if (mode==0){   // wandering blobs
    Serial.printf("Blob age: %d/%d\n", blobs[0]->age, blobs[0]->lifespan);
    Serial.printf("Blob av/cv: %d %d\n", blobs[0]->av, blobs[0]->cv);
    Serial.printf("Blob a/c: %d %d\n", blobs[0]->a, blobs[0]->c);
    Serial.printf("Blob x/y/z: %d %d %d\n", blobs[0]->x(), blobs[0]->y(), blobs[0]->z());
  }
  if (mode==2){  // fade cycle
    Serial.printf("color_mix: %d  power fade: %d\n", color_mix, power_fade);
    CHSV hsv1, hsv2;
    hsv1 = rgb2hsv_approximate(c);
    hsv2 = rgb2hsv_approximate(c2);
    Serial.printf("color1: %d color2: %d\n", hsv1.hue, hsv2.hue);
    Serial.printf("blend1: %d blend2: %d\n", blend1, blend2);
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
    displaySensorDetails();
  }
}
//Ticker timer1;

void setup() {
  float temp = InternalTemperature.readTemperatureC();
  random_seed = (temp - int(temp)) * 100000; 
  random_seed += (analogRead(ANALOG_PIN_A) * analogRead(ANALOG_PIN_B));
  seed1 = random(random_seed) * 2 % 4000;
  seed2 = random(random_seed) * 3 % 5000;
  Serial.begin(115200);
  delay(300);
  Serial.println("Start");

  pinMode(ON_BOARD_LED, OUTPUT);
  pinMode(ANALOG_PIN_A, INPUT);
  pinMode(ANALOG_PIN_B, INPUT);
  pinMode(USER_BUTTON, INPUT_PULLUP);

  Serial.printf("Temp: %f c\n", temp);
  Serial.printf("Random seed: %d\n", random_seed);
  randomSeed(random_seed);
    /* Initialise the sensor */
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
   
  /* Use external crystal for better accuracy */
  bno.setExtCrystalUse(true);
   
  /* Display some basic information on this sensor */
  displaySensorDetails();


  // set up fastled - two strips on two pins
  // see https://github.com/FastLED/FastLED/wiki/Parallel-Output#parallel-output-on-the-teensy-4
  FastLED.addLeds<2, WS2812, WS2812_LED_PIN, GRB>(leds, NUM_LEDS/2);
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

  // inital mode at startup
  mode = 2;
}

#define NUM_MODES 8
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
    mode++;
    mode %= NUM_MODES;
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