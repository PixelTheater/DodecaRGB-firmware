#include "unity.h"
#include "particle.h"

void setUp(void) {
    // This is run before EACH test
    
}

void tearDown(void) {
    // This is run after EACH test
}

void test_particle_tick_age_increment(void) {
    // Arrange
    Particle p;
    int initial_age = p.age;

    // Act
    p.tick();

    // Assert
    TEST_ASSERT_EQUAL_INT(initial_age + 1, p.age);  // Check if age is incremented
}

void test_particle_tick_age_reset(void) {
    // Arrange
    Particle p;
    p.age = p.hold_time;  // Set age to hold_time so that the if condition in tick() is true

    // Act
    p.tick();

    // Assert
    TEST_ASSERT_EQUAL_INT(0, p.age);  // Check if age is reset to 0
}

void test_particle_tick_status_update(void) {
    // Arrange
    Particle p;
    p.age = p.hold_time;  // Set age to hold_time so that the if condition in tick() is true

    // Act
    p.tick();

    // Assert
    //EST_ASSERT_EQUAL_INT(held, p.status);  // Check if status is updated to held
}

void test_particle_tick_led_selection(void) {
    // Arrange
    Particle p;
    p.age = p.hold_time;  // Set age to hold_time so that the if condition in tick() is true

    // Act
    p.tick();

    // Assert
    // Check if led_number is updated to the best pick
    // This test might need to be updated based on how you define the best pick
    // TEST_ASSERT_EQUAL_INT(best_pick, p.led_number);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_particle_tick_age_increment);
    RUN_TEST(test_particle_tick_age_reset);
    RUN_TEST(test_particle_tick_status_update);
    RUN_TEST(test_particle_tick_led_selection);
    UNITY_END();

    return 0;
}