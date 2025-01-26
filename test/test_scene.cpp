#include <doctest/doctest.h>
#include "scene.h"
#include "settings.h"

using namespace Animation;

// Test fixture that exposes protected methods
class TestSceneFixture : public Scene {
public:
    // Expose protected methods for testing
    using Scene::settings;  // Make settings() public
    using Scene::setup;     // Make setup() public
    using Scene::reset;     // Make reset() public
    using Scene::onSettingsChanged;  // Make onSettingsChanged() public
    
    // Required overrides
    void setup() override {
        settings().param("speed")
            .range(0.0f, 1.0f)
            .set(0.5f)
            .build();
            
        settings().param("size")
            .range(0, 10)
            .set(5)
            .build();
            
        settings().param("enabled")
            .boolean()
            .set(true)
            .build();
            
        settings().param("count")
            .range(0, 10)
            .set(5)
            .build();
            
        _speed = settings()("speed");
        _size = settings().operator()<int>("size");
    }
    
    void reset() override {
        _speed = settings()("speed");
        _size = settings().operator()<int>("size");
    }
    
    void tick() override {} // Required by pure virtual
    
    // Test state access
    float getSpeed() const { return _speed; }
    bool getEnabled() const { return settings().operator()<bool>("enabled"); }
    int getCount() const { return settings().operator()<int>("count"); }
    int getSize() const { return _size; }

    void onSettingsChanged() override {
        // Update all values when settings change
        _speed = settings()("speed");
        _size = settings().operator()<int>("size");
    }

private:
    float _speed = 0.0f;
    int _size = 0;
};

TEST_CASE("Scene Lifecycle") {
    TestSceneFixture scene;
    
    SUBCASE("Setup and Reset") {
        scene.setup();
        CHECK(scene.getSpeed() == 0.5f);
        CHECK(scene.getSize() == 5);
    }
    
    SUBCASE("Settings Changes") {
        scene.setup();
        scene.settings().set("speed", 0.8f);
        scene.reset();
        CHECK(scene.getSpeed() == 0.8f);
    }
}

TEST_CASE("Settings Changes") {
    TestSceneFixture scene;
    scene.setup();
    
    SUBCASE("Direct reset") {
        scene.settings().set("speed", 0.8f);
        scene.reset();
        CHECK(scene.getSpeed() == doctest::Approx(0.8f));
        CHECK(scene.getSize() == 5);  // Initial value from setup
    }
    
    SUBCASE("Settings changed notification") {
        scene.settings().set("speed", 0.8f);
        scene.settings().set("size", 20);
        scene.onSettingsChanged();  // Only updates speed
        CHECK(scene.getSpeed() == doctest::Approx(0.8f));
        CHECK(scene.getSize() == 10);  // Unchanged
    }
}

TEST_CASE("Scene Parameter Setup") {
    TestSceneFixture scene;
    scene.setup();  // Use test helper instead of protected setup()
    
    // Use test helpers to check values
    CHECK(scene.getSpeed() == 0.5f);
    CHECK(scene.getEnabled() == true);
    CHECK(scene.getCount() == 5);
    
    // Or use test_settings() if we need direct settings access
    Settings& settings = scene.settings();
    CHECK(settings("speed") == 0.5f);
} 