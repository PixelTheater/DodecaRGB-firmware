#include <Arduino.h>
#include <Wire.h>
#include <InternalTemperature.h>
#include <cmath>
#include <FastLED.h>

#include "points.h"
#include "particle.h"
#include "palettes.h"
#include "animation_manager.h"

#include <Adafruit_LSM6DSOX.h>

#include "models/DodecaRGBv2/model.h"
// or #include "models/IcosahedronV1.h"

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

*/

// LED configs
// Version is defined in VERSION file in root directory
#define VERSION PROJECT_VERSION
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

long random_seed = 0;
int seed1,seed2 = 0;
int mode = 0;



float calculate_power_usage() {
    float unscaled_power = calculate_unscaled_power_mW(leds, NUM_LEDS);
    float brightness_scale = Animation::getBrightness() / 370.0f;
    float base_power = 40.0f;  // Base power consumption in mW
    
    return base_power + (unscaled_power - base_power) * brightness_scale;
}

void timerStatusMessage(){
  // print global info
  Serial.printf("--> mode:%d (%s) @ %d FPS <--\n", 
    mode,
    animation_manager.getCurrentAnimationName().c_str(),
    FastLED.getFPS() 
  );

  String status = animation_manager.getCurrentAnimation()->getStatus();
  Serial.printf("%s\n", status.c_str());

// #ifdef USE_IMU
//     // Display sensor data
//     Serial.printf("LSM6DSOX:\n");
//     Serial.printf("\tTemperature: %.2f deg C\n", temp.temperature);
//     Serial.printf("\tAccel X: %.2f \tY: %.2f \tZ: %.2f m/s^2\n", 
//       accel.acceleration.x, accel.acceleration.y, accel.acceleration.z);
//     Serial.printf("\tGyro X: %.2f \tY: %.2f \tZ: %.2f radians/s\n", 
//       gyro.gyro.x, gyro.gyro.y, gyro.gyro.z);
// #endif

  Serial.printf("Est Power: %0.1f W (%.1f%% brightness)\n", 
    calculate_power_usage()/1000.0,
    (Animation::getBrightness() / 255.0f) * 100);
}

void fadeInSide(int side, int start_led, int end_led, int duration_ms) {
    for (int brightness = 0; brightness <= 120; brightness += 30) {
        for (int led = start_led; led <= end_led; ++led) {
            int index = side * LEDS_PER_SIDE + led;
            CRGB side_color = ColorFromPalette(RainbowColors_p, side * 255 / NUM_SIDES);
            leds[index] = side_color;
            leds[index].fadeToBlackBy(255 - brightness);
        }
        FastLED.show();
        delay(duration_ms);  // Adjust delay to complete fade-in within the specified duration
    }
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

  // Fade in each side in sequence
  int fade_duration = 1;  
  for (int side = 0; side < NUM_SIDES; ++side) {
    fadeInSide(side, 6, 15, fade_duration);
  }

  Serial.println("Init done");

  FastLED.clear();
  FastLED.show();

  delay(100);

  // Add animations with default settings (order defines sequence)

  // animation_manager.add("identify_sides");
  animation_manager.add("boids");
  animation_manager.add("blobs");
  animation_manager.add("xyz_scanner");  
  animation_manager.add("sparkles");
  animation_manager.add("colorshow");
  animation_manager.add("wandering_particles");
  animation_manager.add("geography");
  animation_manager.add("identify_sides");
  animation_manager.add("orientation_demo");


  // Configure animation presets
  animation_manager.preset("sparkles", "default");
  animation_manager.preset("xyz_scanner", "fast");  // Try different speeds
  animation_manager.preset("blobs", "fast");

  // animation_manager.setPlaybackMode(PlaybackMode::ADVANCE, 15.0f);  // will changes animations every 15 seconds
  // animation_manager.setPlaybackMode(PlaybackMode::RANDOM, 60.0f);  // randomly select a new animation every 60 seconds
  animation_manager.setPlaybackMode(PlaybackMode::HOLD);  // stays on the current animation until the button is pressed

  // set initial animation
  animation_manager.setCurrentAnimation("boids");

  auto model = std::make_unique<Models::DodecaRGBv2>();
  stage.setModel(std::move(model));
}

long interval, last_interval = 0;
const long max_interval = 3000;

void updateOnboardLED() {
    static uint8_t led_brightness = 0;
    static uint32_t last_update = 0;
    const uint32_t update_interval = 16;  // ~60Hz updates
    
    interval = millis()/max_interval;
    if (millis() - last_update >= update_interval) {
        // Create smooth sine wave breathing (4 second cycle)
        float breath = (sin(millis() * PI / 2000.0) + 1.0) / 2.0;
        led_brightness = breath * 255;
        analogWrite(ON_BOARD_LED, led_brightness);
        last_update = millis();
    }
}

void loop() {  
  updateOnboardLED();  // Replace old LED code with this
  
  if (interval != last_interval){
    timerStatusMessage();
    last_interval = interval;
  }
  // handle button press for mode change
  if (digitalRead(USER_BUTTON) == LOW){
    // flash while button is still down
    while (digitalRead(USER_BUTTON) == LOW){
      CRGB c = CRGB::White;
      c.setHSV(millis()/500 % 255, 255, 64);
      FastLED.showColor(c);
      FastLED.show(); 
      delay(20); 
    }
    Serial.println("Button released");
    mode = (mode + 1) % animation_manager.getPlaylistLength();
    Serial.printf("Button pressed, changed mode to %d\n", animation_manager.getCurrentAnimationIndex());
    animation_manager.nextAnimation();
  }
  animation_manager.update();
  FastLED.show();
}
