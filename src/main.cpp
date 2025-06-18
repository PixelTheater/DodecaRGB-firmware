#include <Arduino.h>
#include <Wire.h>
#include <InternalTemperature.h>
#include <cmath>
#include <FastLED.h>
#include <memory>

// PixelTheater includes
#include "PixelTheater.h" // Use the consolidated header

// Include the model definition (for use in useFastLEDPlatform template)
#ifndef DODECARGBV2_1_MODEL_INCLUDED
#define DODECARGBV2_1_MODEL_INCLUDED
#include "models/DodecaRGBv2_1/model.h" 
#endif

// Include Scene implementations 
// #include "scenes/blob_scene.h" // Temporarily disabled until refactored <-- REMOVE COMMENT
#include "scenes/identify_sides/identify_sides_scene.h" // ADDED IdentifySidesScene
#include "scenes/satellites/SatellitesScene.h" // <<< ADDED Satellites Scene
#include "scenes/blobs/blob_scene.h" // Refactored
#include "scenes/xyz_scanner/xyz_scanner_scene.h" // Refactored
#include "scenes/wandering_particles/wandering_particles_scene.h" // Refactored
#include "scenes/boids/boids_scene.h" // Refactored
#include "scenes/test_scene/test_scene.h" // Include Test Scene
#include "scenes/geography/geography_scene.h" // Include the new scene
#include "scenes/orientation_grid/orientation_grid_scene.h" // ADDED
#include "scenes/sparkles/sparkles_scene.h" // UPDATED
#include "scenes/texture_map/texture_map_scene.h" // ADDED NEW SCENE
#include "benchmark.h" 

#ifndef PROJECT_VERSION
#define PROJECT_VERSION "0.2.2"
#endif

// LED configs
// Version is defined in VERSION file in root directory
#define VERSION PROJECT_VERSION
#define USER_BUTTON 2
// https://github.com/FastLED/FastLED/wiki/Parallel-Output#parallel-output-on-the-teensy-4
// 4 pins are used to control 4 strips of 312 LEDs, for a total of 1248 LEDs
// Teensy 4.1/fastled pin order: .. ,19,18,14,15,17, ..
#define LED_CHANNEL_1_PIN 19  
#define LED_CHANNEL_2_PIN 18  
#define LED_CHANNEL_3_PIN 14  
#define LED_CHANNEL_4_PIN 15  
#define ANALOG_PIN_A 24
#define ANALOG_PIN_B 25
#define ON_BOARD_LED 13

// Teensy I2C on pins 17/16 (SDA,SCL) is mapped to Wire1 in Arduinio framework
// https://www.pjrc.com/teensy/td_libs_Wire.html
// ex: Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29, &Wire1);

#define BRIGHTNESS  15      // global brightness, should be used by all animations
#define USE_IMU true        // enable orientation sensor (currently: LSM6DSOX)

// model settings (replace with generated model params)
#define NUM_LEDS 1620
#define NUM_SIDES 12
#define LEDS_PER_SIDE 135

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
  // GPIO 19, 18, 14, 15 are used on the DodecaRGB Controller PCB
  int leds_per_strip = NUM_LEDS/4;
  FastLED.addLeds<WS2812, LED_CHANNEL_1_PIN, GRB>(leds, NUM_LEDS/4);
  FastLED.addLeds<WS2812, LED_CHANNEL_2_PIN, GRB>(leds, NUM_LEDS/4, NUM_LEDS/4);
  FastLED.addLeds<WS2812, LED_CHANNEL_3_PIN, GRB>(leds, NUM_LEDS/4*2, NUM_LEDS/4);
  FastLED.addLeds<WS2812, LED_CHANNEL_4_PIN, GRB>(leds, NUM_LEDS/4*3, NUM_LEDS/4);

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
  theater.useFastLEDPlatform<PixelTheater::Models::DodecaRGBv2_1>(
    leds,
    NUM_LEDS
  );
  
  // Validate model integrity after initialization
  Serial.println("Validating model geometry and data integrity...");
  if (auto* model = theater.model()) {
    auto validation = model->validate_model(true, true);  // Both geometric and data validation
    
    if (validation.is_valid) {
      Serial.printf("✓ Model validation passed: %d/%d checks successful\n", 
        validation.total_checks - validation.failed_checks, validation.total_checks);
    } else {
      Serial.printf("✗ Model validation FAILED: %d/%d checks failed\n", 
        validation.failed_checks, validation.total_checks);
      
      // Log specific validation errors
      if (!validation.data_integrity.face_ids_unique) {
        Serial.println("  ERROR: Face IDs are not unique");
      }
      if (!validation.data_integrity.led_indices_sequential) {
        Serial.println("  ERROR: LED indices are not sequential");
      }
      if (!validation.data_integrity.edge_data_complete) {
        Serial.println("  ERROR: Edge data is incomplete");
      }
      if (!validation.data_integrity.vertex_data_complete) {
        Serial.println("  ERROR: Vertex data is incomplete");
      }
      if (!validation.data_integrity.indices_in_bounds) {
        Serial.println("  ERROR: Indices are out of bounds");
      }
      if (!validation.geometric.all_faces_planar) {
        Serial.println("  ERROR: Not all faces are planar");
      }
      if (!validation.geometric.all_leds_within_faces) {
        Serial.println("  ERROR: Some LEDs are outside face boundaries");
        
        // Debug: Show face geometry vs LED positions for first face
        Serial.println("  DEBUG: Face 0 geometry analysis:");
        const auto& face0 = model->face(0);
        const auto& vertices = face0.vertices;  // vertices is a member, not a method
        
        // Show face vertices
        Serial.println("    Face vertices:");
        for (size_t i = 0; i < vertices.size() && i < 5; i++) {
          const auto& v = vertices[i];
          Serial.printf("      V%d: (%.1f, %.1f, %.1f)\n", i, v.x, v.y, v.z);
        }
        
        // Calculate and show face center
        float center_x = 0, center_y = 0, center_z = 0;
        for (size_t i = 0; i < vertices.size(); i++) {
          center_x += vertices[i].x;
          center_y += vertices[i].y; 
          center_z += vertices[i].z;
        }
        center_x /= vertices.size();
        center_y /= vertices.size();
        center_z /= vertices.size();
        Serial.printf("    Face center: (%.1f, %.1f, %.1f)\n", center_x, center_y, center_z);
        
        // Calculate face radius
        float max_vertex_distance = 0;
        for (size_t i = 0; i < vertices.size(); i++) {
          const auto& v = vertices[i];
          float dx = v.x - center_x;
          float dy = v.y - center_y; 
          float dz = v.z - center_z;
          float distance = sqrt(dx*dx + dy*dy + dz*dz);
          max_vertex_distance = max(max_vertex_distance, distance);
        }
        Serial.printf("    Face radius: %.1f, Tolerance: %.1f\n", max_vertex_distance, max_vertex_distance * 2.0f);
        
        // Show first few LED positions and distances
        Serial.println("    LED positions (first 10):");
        for (size_t i = 0; i < 10 && i < face0.led_count(); i++) {
          // Access LED through point interface  
          const auto& led_point = model->point(face0.led_offset() + i);
          float led_dx = led_point.x() - center_x;
          float led_dy = led_point.y() - center_y;
          float led_dz = led_point.z() - center_z;
          float distance = sqrt(led_dx*led_dx + led_dy*led_dy + led_dz*led_dz);
          bool within_tolerance = distance <= (max_vertex_distance * 2.0f);
          Serial.printf("      LED%d: (%.1f, %.1f, %.1f) dist=%.1f %s\n", 
            i, led_point.x(), led_point.y(), led_point.z(), distance,
            within_tolerance ? "✓" : "✗");
        }
      }
      if (!validation.geometric.vertex_coordinates_sane) {
        Serial.println("  ERROR: Vertex coordinates are not reasonable");
      }
      if (!validation.geometric.led_coordinates_sane) {
        Serial.println("  ERROR: LED coordinates are not reasonable");
      }
      if (!validation.geometric.edge_connectivity_complete) {
        Serial.println("  ERROR: Edge connectivity is broken");
      }
      
      // Print detailed error messages
      for (size_t i = 0; i < validation.errors.error_count && i < 10; i++) {  // Limit to first 10 errors
        Serial.printf("  DETAIL: %s\n", validation.errors.error_messages[i]);
      }
      if (validation.errors.error_count > 10) {
        Serial.printf("  ... and %d more errors\n", validation.errors.error_count - 10);
      }
      
      Serial.println("WARNING: Proceeding with potentially invalid model data!");
    }
  } else {
    Serial.println("ERROR: Cannot validate model - model pointer is null!");
  }
  
  // Add scenes 
  //theater.addScene<Scenes::TestScene>(); // Add Test Scene first
  theater.addScene<Scenes::IdentifySidesScene>(); // ADDED IdentifySidesScene
  theater.addScene<Scenes::SparklesScene>(); // UPDATED
  theater.addScene<Scenes::SatellitesScene>(); // <<< ADDED Satellites Scene
  theater.addScene<Scenes::WanderingParticlesScene>(); // Add Wandering Particles
  theater.addScene<Scenes::TextureMapScene>(); // TextureMapScene moved to Scenes namespace
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
  
  // Check interval timer *before* update, but log *after*
  bool log_status_this_frame = (interval != last_interval);
  if (log_status_this_frame) {
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
    // Immediately log status after scene change
    timerStatusMessage(); 
    log_status_this_frame = false; // Don't log again immediately
  }

  // Update the Theater (calls current scene's tick() and platform->show())
  BENCHMARK_START("frame_total");
  theater.update();
  BENCHMARK_END();

  // Log status *after* the update, if the interval passed
  if (log_status_this_frame) {
    timerStatusMessage();
  }
}
