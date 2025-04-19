#include <Arduino.h>
#include <Wire.h>
#include <InternalTemperature.h>
#include <cmath>
#include <FastLED.h>
#include <memory>

// PixelTheater includes
#include "PixelTheater.h" // Use the consolidated header

// Include the model definition (for use in useFastLEDPlatform template)
#ifndef DODECARGBV2_MODEL_INCLUDED
#define DODECARGBV2_MODEL_INCLUDED
#include "models/DodecaRGBv2/model.h" 
#endif

// Include Scene implementations 
// #include "scenes/blob_scene.h" // Temporarily disabled until refactored <-- REMOVE COMMENT
#include "scenes/blobs/blob_scene.h" // Refactored
#include "scenes/xyz_scanner/xyz_scanner_scene.h" // Refactored
#include "scenes/wandering_particles/wandering_particles_scene.h" // Refactored
#include "scenes/boids/boids_scene.h" // Refactored
#include "scenes/test_scene/test_scene.h" // Include Test Scene
#include "scenes/geography/geography_scene.h" // Include the new scene
#include "scenes/orientation_grid/orientation_grid_scene.h" // ADDED
#include "scenes/sparkles/sparkles.h" // ADDED
#include "scenes/texture_map/texture_map_scene.h" // ADDED NEW SCENE
#include "benchmark.h" 

#ifndef PROJECT_VERSION
#define PROJECT_VERSION "0.2.0"
#endif

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

#define BRIGHTNESS  15      // global brightness, should be used by all animations
#define USE_IMU true        // enable orientation sensor (currently: LSM6DSOX)

// model settings (replace with generated model params)
#define NUM_LEDS 1248
#define NUM_SIDES 12
#define LEDS_PER_SIDE 104

::CRGB leds[NUM_LEDS];

PixelTheater::Theater theater; // Global Theater instance

long random_seed = 0;
int seed1,seed2 = 0;
int mode = 0;

float calculate_power_usage() {
  // todo
  return 0.0f;
}

void timerStatusMessage(){
  // uint8_t scene_number = 0; // TODO: Get current scene index from Theater?
  size_t scene_number = 0; // Default to 0 if no scene
  String scene_name = "Unknown";
  std::string scene_status = "(No Scene)"; // Default status
  
  // Get current scene name and status from Theater
  PixelTheater::Scene* current = theater.currentScene();
  if (current) { 
    scene_name = current->name().c_str(); 
    scene_status = current->status(); // Get status from the current scene
    if (scene_status.empty()) { scene_status = "(empty)"; } // Handle empty status
    
    // Find the index of the current scene
    const auto& all_scenes = theater.scenes();
    for(size_t i = 0; i < all_scenes.size(); ++i) {
        if (all_scenes[i].get() == current) {
            scene_number = i;
            break;
        }
    }
  }
  
  Serial.printf("--> mode:%d (%s) @ %d FPS <--\n", 
    scene_number,
    scene_name.c_str(),
    FastLED.getFPS() 
  );

  // Print the fetched scene status
  Serial.printf("Status: %s\n", scene_status.c_str());

  Serial.printf("Est Power: %0.1f W (%.1f%% brightness)\n", 
    calculate_power_usage()/1000.0,
    (BRIGHTNESS/ 255.0f) * 100);
    
  BENCHMARK_REPORT(FastLED.getFPS());
}

void fadeInSide(int side, int start_led, int end_led, int duration_ms) {
    for (int brightness = 0; brightness <= 120; brightness += 30) {
        for (int led = start_led; led <= end_led; ++led) {
            int index = side * LEDS_PER_SIDE + led;
            ::CRGB side_color = ColorFromPalette(RainbowColors_p, side * 255 / NUM_SIDES);
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

  // Initialize PixelTheater Theater
  Serial.println("Initializing PixelTheater Theater...");
  Benchmark::enabled = true;  

  // Initialize Theater with FastLED platform and specific model
  theater.useFastLEDPlatform<PixelTheater::Models::DodecaRGBv2>(
    leds,
    NUM_LEDS
  );
  
  // Add scenes 
  //theater.addScene<Scenes::TestScene>(); // Add Test Scene first
  theater.addScene<Scenes::WanderingParticlesScene>(); // Add Wandering Particles
  theater.addScene<PixelTheater::TextureMapScene>(); // ADDED NEW SCENE
  theater.addScene<Scenes::Sparkles>(); // ADDED
  theater.addScene<Scenes::OrientationGridScene>(); // ADDED
  theater.addScene<Scenes::BlobScene>(); 
  theater.addScene<Scenes::XYZScannerScene>(); 
  theater.addScene<Scenes::BoidsScene>(); // Add Boids Scene
  theater.addScene<Scenes::GeographyScene>(); // Add the new scene instance
  
  // Start the theater 
  theater.start();

  Serial.println("Init done");

  FastLED.clear();
  FastLED.show();

  delay(100);
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
  updateOnboardLED();  
  
  if (interval != last_interval){
    timerStatusMessage();
    last_interval = interval;
  }
  
  // handle button press for mode change
  if (digitalRead(USER_BUTTON) == LOW){
    while (digitalRead(USER_BUTTON) == LOW){
      ::CRGB c = ::CRGB::White;
      c.setHSV(millis()/500 % 255, 255, 64);
      FastLED.showColor(c);
      FastLED.show(); 
      delay(20); 
    }
    Serial.println("Button released");
    mode = (mode + 1); // Simple counter for now
    Serial.printf("Button pressed, advancing scene...\n");
    BENCHMARK_RESET();
    theater.nextScene(); // Use Theater to switch scene
  }

  // Update the Theater (calls current scene's tick() and platform->show())
    BENCHMARK_START("frame_total");
  theater.update();
    BENCHMARK_END();
}
