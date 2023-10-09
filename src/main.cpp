#include <Arduino.h>
#include <cmath>
#include "FastLED.h"
#include "Ticker.h"
#include "points.h"
#include "network.h"
#include "blob.h"
#include "particle.h"

/*

We have a dodecahedron model with 12 sides. 
Each side is a pentgon-shaped PCB circuit board with RGB LEDs arranged on them, spaced evenly. 

An ESP32 is used to control the LEDs, using the Arduino environment and FastLED library.

Each pentagon side contains 26 RGB leds on a circuit board, arranged in the following way:
- 1 central LED in the exact middle of each side
- a ring of 10 LEDs in a circle around the central LED
- 15 LEDs around the edges, in groups of 3, spaced evenly around the edge of the pentagon

The LEDs on each face of the dodecahedron are wired in series so that the central LED is first, 
the ring LEDs come next, and the edge LEDs after that. 

Each side connects to the next, in series, for a grand total of 312 LEDs. 

As the PCB circuit boards are wired together to form the dodecahedron, the arrangement of the sides must be 
defined, as there are many possible configurations. 
In addition, the rotation of each side must be defined, as it can have five possible rotations.

There's a processing sketch at https://github.com/somebox/dodeca-rgb-simulator that generates the list
of points, the X,Y,Z coordinates, and defines the order of the sides and their rotations. It also
renders an interactive 3D model of the dodecahedron.

*/

// LED configs
#define BRIGHTNESS  40
#define LED_PIN 5
#define WIFI_ENABLED false

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

CRGB leds[NUM_LEDS];
#define USER_BUTTON 0

// Constants
const int LEDS_PER_RING = 10;
const int LEDS_PER_EDGE = 15;

int mode = 0;

void color_show(){
  // light up all LEDs in sequence
  CRGB color = CRGB::White;
  int c = random(1,255-NUM_LEDS);
  for (int i = 0; i < NUM_LEDS; i++) {
    color.setHSV(c+i, 255, 128);
    leds[i] = color;
    if (digitalRead(0) == LOW){
      return;
    } else {
      delay(20);
      FastLED.show();
    }
  }
  delay(100);
  // turn off all LEDs in a dissolving pattern
  for (int x=0; x<50; x++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].fadeToBlackBy(3+random(5));
    }
    if (digitalRead(0) == LOW){
      return;
    } else {
      FastLED.show();
      delay(50);
    }
  }
}

void solid_sides(){
  static int s=0;
  if (random(12)==0) { s = random(NUM_SIDES); };
  CRGB c = CHSV(millis()/random(100,110) % 255, 250, 220);
  for (int level=0; level<50; level+=1){
    for (int i=s*LEDS_PER_SIDE; i<(s+1)*LEDS_PER_SIDE; i++){
      nblend(leds[i], c, 6);
    }
    FastLED.show();
    if (digitalRead(0) == LOW) return;
    delayMicroseconds(100);
  } 
  for (int level=100; level>0; level--){
    for (int i=(s)*LEDS_PER_SIDE; i<(s+1)*LEDS_PER_SIDE; i++){
      nblend(leds[i], CHSV(millis()/random(100,110) % 255, 250, random(50)+150), 1);
    }
    FastLED.show();
    if (digitalRead(0) == LOW) return;
    delayMicroseconds(100);
  }
  FastLED.show();
  s = (s+1) % NUM_SIDES;
}

// randomly light up LEDs and fade them out individually, like raindrops
int fade_level = 0;
void flash_fade_points(){
  // cycle color over time
  CRGB c = CRGB(
    sin8_C((millis()/500)%255),
    sin8_C((millis()/400)%255),
    sin8_C((millis()/300)%255)
  ); 
  fade_level = sin8_C((millis()/600)%255);
  for (int n=0; n<12; n++){
    // central LED
    if (random(35) == 0) nblend(leds[n*LEDS_PER_SIDE], c, map(fade_level, 0, 255, 8, 30));
    // ring LEDs
    if (random(4) == 0) {
      int r1 = random(n*LEDS_PER_SIDE+1, (n+1)*LEDS_PER_SIDE+10);        
      nblend(leds[r1], c, map(fade_level, 0, 255, 30, 60));
    }
    // edge LEDs
    if (random(2) == 0) {
      int r2 = random(n*LEDS_PER_SIDE+11, (n+1)*LEDS_PER_SIDE+26);
      nblend(leds[r2], c,map(fade_level, 0, 255, 30, 60));  
    }
  }
  for (int i = 0; i < NUM_LEDS; i++){
    if (random(350) < fade_level+2) leds[i].fadeToBlackBy(map(fade_level, 0, 255, 50, 30));
  }
  FastLED.show();   
  delay(1);  
}


int calculateBlobDistance(LED_Point p1, Blob *b) {
  int dx = p1.x - b->x();
  int dy = p1.y - b->y();
  int dz = p1.z - b->z();
  return (dx*dx + dy*dy + dz*dz);  
}

#define NUM_BLOBS 4
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
        if (blobs[b]->age < 255){
          c.fadeToBlackBy(255 - blobs[b]->age);
        }
        // leds[i] += c;
        nblend(leds[i], c, map(dist, 0, rad_sq, 180, 30));
        // leds[i] = CHSV(blobs[b]->color, 255, 255-dist);
      }
    }
  }

  for (int i=0; i<NUM_LEDS; i++){
    leds[i].fadeToBlackBy(25);
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
  float speed = 0.02;
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


#define NUM_PARTICLES 15
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
    Serial.printf("Fade level: %d\n", fade_level);  
  }
  if (mode==4){
    // wandering particles
    Serial.printf("pos: %f,%f,%f\n", particles[0]->x(), particles[0]->y(), particles[0]->z());
    Serial.printf("a/c: %f,%f\n", particles[0]->a, particles[0]->c);
    Serial.printf("av/cv: %f,%f\n", particles[0]->av, particles[0]->cv);
  }
}
Ticker timer1;


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
    if (particles[p]->z() > 260){
      reset_particle(particles[p]);
    }
  }
  for (int i=0; i<NUM_LEDS; i++){
    if (random(100)<10) continue;
    leds[i].fadeToBlackBy(8);
  }
  FastLED.show();
  delay(2);
}


void drip_particles(){
  // todo
}


void setup() {
  // set up fastled
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setDither(0);
  FastLED.clear();
  FastLED.show();

  for (int side=0; side<NUM_SIDES; side++){
    leds[side*LEDS_PER_SIDE] = my_colors[side];
  }
  FastLED.show();

  delay(500);
  Serial.println("Start");

  // init Points
  for (int p=0; p < NUM_LEDS; p++){
    points[p].find_nearest_leds();
  }

  // init Blobs
  for (int b=0; b<NUM_BLOBS; b++){
    blobs[b] = new Blob();
    blobs[b]->color = CRGB(random(255), random(255), random(255));
  }
  // init Particles
  int p_step = 280/NUM_PARTICLES;
  for (int p=0; p<NUM_PARTICLES; p++){
    particles[p] = new Particle();
    reset_particle(particles[p]);
  }

  pinMode(0, INPUT_PULLUP);

  Serial.println("Init done");

  for (int i=1; i<11; i++){    
    for (int side=0; side<NUM_SIDES; side++){
      leds[side*LEDS_PER_SIDE+i] = my_colors[side];
    }
    FastLED.show();
    delay(50);
  }

  if (WIFI_ENABLED == true){
    bool config_wifi = (digitalRead(USER_BUTTON) == LOW);
    bool connected = ConnectToWifi(config_wifi);

    // flash green if connected, or red if not
    for (int x=0; x<40; x++){
      int level = 150 - abs(map(x, 0,  40, -150, 150));
      CRGB c =  (connected ? CRGB(0,level,0) : CRGB(level,0,0));
      FastLED.showColor(c);
      FastLED.show();
      delayMicroseconds(500);
    }
  } else {
    Serial.println("Wifi disabled.");
  }

  FastLED.setDither(0);
  FastLED.clear();
  FastLED.show();

  delay(300);

  timer1.attach(3, timerStatusMessage);

  mode = 0;

}



void loop() {
  // handle button press for mode change
  if (digitalRead(USER_BUTTON) == LOW){
    Serial.print("Button pressed, changing mode to ");
    mode = (mode + 1) % 5;
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
  }
  if (mode == 1){
    fade_test();
  }
  if (mode == 2){
    flash_fade_points();
  }
  if (mode==3){
    solid_sides();
  }
  if (mode==4){
    wandering_particles();
    // drip_particles();
  }
}