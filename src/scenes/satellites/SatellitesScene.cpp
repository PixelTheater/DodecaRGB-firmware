#include "SatellitesScene.h"
#include <cmath>
#include "benchmark.h" // Include benchmark header
#include <vector> // Need for std::vector
#include <limits>   // Required for numeric_limits
#include <algorithm> // Required for std::sort, std::min/max
#include <utility>  // Required for std::pair
// #include <Eigen/Geometry> // Removed - Not available

namespace Scenes {

// Define constants
const float SatellitesScene::BASE_ORBITAL_RADIUS = 100.0f; 
const float SatellitesScene::BASE_ANGULAR_SPEED = 0.7f; 
const float SatellitesScene::RESPAWN_DELAY = 20.0f;
const float SatellitesScene::SPAWN_DURATION = 1.8f;
const float SatellitesScene::CRASH_DURATION = 0.75f;
const float SatellitesScene::MAX_FADE_AMOUNT = 48.0f;
const float SatellitesScene::CHAOS_FORCE_SCALE = 0.5f; 
const float SatellitesScene::COLLISION_PROXIMITY = 5.0f;
// New Spark Constants
const int   SatellitesScene::MIN_SPARKS_PER_COLLISION = 4;
const int   SatellitesScene::MAX_SPARKS_PER_COLLISION = 7;
const float SatellitesScene::SPARK_BASE_SPEED = 30.0f;
const float SatellitesScene::MIN_SPARK_LIFETIME = 1.4f;
const float SatellitesScene::MAX_SPARK_LIFETIME = 2.5f;
const uint8_t SatellitesScene::SPARK_BLEND_AMOUNT = 230;

// Global speed factor
const float TIME_SCALE_FACTOR = 0.8f; 

void SatellitesScene::setup() {
    set_name("Satellites");
    set_description("Satellites orbiting on the surface, crashing on collision.");

    // Simplified Parameters
    param("population", "count", 1, 200, 22);
    param("speed", "range", 0.1f, 5.0f, 1.6f); // Scales base angular speed
    param("chaos", "ratio", 0.0f, 1.0f, 0.15f); // How much orbits are perturbed
    param("trails", "ratio", 0.6f); // Fade amount (higher = less fade)
    param("render_radius", "range", 0.01f, 0.3f, 0.083f); // Satellite head angular size
    param("blur", "ratio", 0.0f, 1.0f, 0.0f); // Post-process spatial blur

    // Init satellites (Dead with random timers)
    int population = settings["population"];
    satellites.resize(population);
    for (auto& sat : satellites) {
        sat.timer = randomFloat(0.0f, SPAWN_DURATION + RESPAWN_DELAY);
    }
}

void SatellitesScene::tick() {
    Scene::tick();  // Handle base timing
    BENCHMARK_START("scene_total"); // Benchmark the whole tick
    
    // Apply time scale factor to deltaTime for faster simulation
    const float dt = deltaTime() * TIME_SCALE_FACTOR;
    
    // Read relevant settings once per tick
    const float chaosSetting = static_cast<float>(settings["chaos"]);
    const uint8_t fadeAmount = static_cast<uint8_t>(static_cast<float>(settings["trails"]) * MAX_FADE_AMOUNT);
    const float blurAmount = settings["blur"];
    // 1. Apply fade effect
    BENCHMARK_START("fade_leds");
    if (fadeAmount > 0) {
        for (size_t i = 0; i < ledCount(); ++i) {
            leds[i].fadeToBlackBy(fadeAmount);
        }
    } else {
        // Clear LEDs if no trails
        for (size_t i = 0; i < ledCount(); ++i) {
            leds[i] = CRGB::Black;
        }
    }
    BENCHMARK_END(); // End fade_leds
    
    // 2. Update satellite state (includes path update)
    BENCHMARK_START("update_sat_state");
    for (auto& sat : satellites) {
        updateSatelliteState(sat, dt);
    }
    BENCHMARK_END(); // End update_sat_state
    
    // 3. Apply satellite interactions
    BENCHMARK_START("apply_interactions");
    if (chaosSetting > 0.0f) {
        applySatelliteInteractions(chaosSetting);
    }
    BENCHMARK_END(); // End apply_interactions
    
    // 4. Render satellites (Now split into sub-benchmarks)
    renderSatellites();

    // 5. Update and Render Spark Particles
    BENCHMARK_START("update_render_sparks");
    float renderAngleSpark = static_cast<float>(settings["render_radius"]) * 0.5f; 
    CRGB finalSparkColor = CRGB::Yellow;
    // const float SPARK_DAMAGE_PROXIMITY_SQ = SPARK_DAMAGE_PROXIMITY * SPARK_DAMAGE_PROXIMITY; // Removed - Constant and logic removed

    // Iterate backwards for safe removal
    for (int i = sparkParticles.size() - 1; i >= 0; --i) {
        auto& spark = sparkParticles[i];
        spark.lifetime -= dt;
        if (spark.lifetime <= 0) {
            sparkParticles.erase(sparkParticles.begin() + i);
            continue; 
        }
        spark.position += spark.velocity * dt;

        // Update color (Red to Yellow fade)
        float fadeProgress = 0.0f;
        if (spark.initialLifetime > 1e-6f) { 
             fadeProgress = std::max(0.0f, spark.lifetime / spark.initialLifetime);
        }
        spark.color = blend(finalSparkColor, CRGB::Red, static_cast<uint8_t>(fadeProgress * 255.0f));

        // Render the spark (Optimized: Find single closest LED)
        float minDistSq = 1e18f; // Initialize with a large value
        int closestLedIndex = -1;
        size_t numLeds = ledCount();

        for (size_t led_idx = 0; led_idx < numLeds; ++led_idx) {
            const auto& p = model().point(led_idx);
            float dx = p.x() - spark.position.x();
            float dy = p.y() - spark.position.y();
            float dz = p.z() - spark.position.z();
            float distSq = dx * dx + dy * dy + dz * dz;

            if (distSq < minDistSq) {
                minDistSq = distSq;
                closestLedIndex = static_cast<int>(led_idx);
            }
        }

        // Render to the closest LED found
        if (closestLedIndex >= 0) {
            // Simple blend, maybe adjust amount based on distance later if needed
            uint8_t blendAmount = 180; // Strong blend for the single spark point
            nblend(leds[closestLedIndex], spark.color, blendAmount);
        }
    }
    BENCHMARK_END(); // End update_render_sparks
    
    // 6. Apply Spatial Blur
    BENCHMARK_START("spatial_blur");
    if (blurAmount > 1e-3f && ledCount() > 0) { 
        uint8_t blurAmountInt = static_cast<uint8_t>(blurAmount * 255.0f);

        // Create a temporary copy of the LED buffer
        // Correct way to copy from LedsProxy
        std::vector<CRGB> leds_copy(ledCount()); // Create vector of the right size
        for (size_t k=0; k < ledCount(); ++k) { // Copy element by element
            leds_copy[k] = leds[k];
        }

        for (size_t i = 0; i < ledCount(); ++i) {
            const auto& neighbors = model().point(i).getNeighbors();
            uint8_t neighbor_count = 0;
            uint32_t avg_r = 0, avg_g = 0, avg_b = 0;

            // Calculate average neighbor color from the copy
            // Iterate through the fixed-size neighbor array
            for (const auto& neighbor_info : neighbors) {
                // Check if neighbor ID is valid (within bounds)
                // Assuming neighbor_info.id is set to an invalid value (e.g. >= ledCount() or a specific const)
                // if ID is invalid. Let's just check bounds.
                if (neighbor_info.id < ledCount()) {
                    avg_r += leds_copy[neighbor_info.id].r;
                    avg_g += leds_copy[neighbor_info.id].g;
                    avg_b += leds_copy[neighbor_info.id].b;
                    neighbor_count++;
                }
            }

            if (neighbor_count > 0) {
                CRGB avgNeighborColor(
                    static_cast<uint8_t>(avg_r / neighbor_count),
                    static_cast<uint8_t>(avg_g / neighbor_count),
                    static_cast<uint8_t>(avg_b / neighbor_count)
                );
                // Blend the *original* LED color towards the neighbor average
                nblend(leds[i], avgNeighborColor, blurAmountInt);
            }
        }
    }
    BENCHMARK_END(); // End spatial_blur
    // --- End Spatial Blur ---

    BENCHMARK_END(); // End scene_total
}

void SatellitesScene::updateSatelliteState(Satellite& sat, float dt) {
    float speedSetting = static_cast<float>(settings["speed"]);
    float scaled_dt = dt * speedSetting; 
    float chaosSetting = static_cast<float>(settings["chaos"]);

    switch (sat.state) {
        case Satellite::State::Dead:
            sat.timer -= dt; 
            if (sat.timer <= 0) {
                initializeSatellite(sat);
                sat.state = Satellite::State::Spawning;
                sat.timer = SPAWN_DURATION;
            }
            break;

        case Satellite::State::Spawning:
            sat.timer -= dt; 
            {
                float spawnProgress = 1.0f - (sat.timer / SPAWN_DURATION); // 0 -> 1 linear
                // Apply easing (quadratic in) for slower start, faster end
                spawnProgress = spawnProgress * spawnProgress; 
                // Update path with scaled dt based on progress
                // updateSatellitePath(sat, scaled_dt, chaosSetting * 0.1f); // Old: Full speed path update
                updateSatellitePath(sat, scaled_dt * spawnProgress, chaosSetting * 0.1f); // Scaled speed path update
            }
            if (sat.timer <= 0) {
                sat.state = Satellite::State::Orbiting;
                sat.timer = 0.0f;
            }
            break;
            
        case Satellite::State::Orbiting:
            updateSatellitePath(sat, scaled_dt, chaosSetting);
            // No health/crash checks here anymore
            break;
            
        case Satellite::State::Crashing:
            sat.timer -= dt; 
            if (sat.timer <= 0) {
                sat.state = Satellite::State::Dead;
                sat.timer = RESPAWN_DELAY;
                sat.position = Eigen::Vector3f::Zero();
                break; 
            }
            {
                 float crashProgress = 1.0f - (sat.timer / CRASH_DURATION); // 0.0 -> 1.0
                 
                 // Slow down path update cubically, with randomness
                 float speedFactorBase = (1.0f - crashProgress);
                 speedFactorBase = speedFactorBase * speedFactorBase * speedFactorBase; // Cubic slowdown
                 float speedFactor = speedFactorBase * randomFloat(0.7f, 1.3f);
                 speedFactor = std::max(0.0f, std::min(1.0f, speedFactor)); 
                 updateSatellitePath(sat, scaled_dt * speedFactor, chaosSetting * 0.5f); 

                 // Visually pull position towards origin based on crash progress
                 // Scale factor goes from 1 (start) down to 0 (end)
                 float shrinkFactor = 1.0f - crashProgress;
                 // Apply easing (e.g. quadratic in) to make it accelerate inwards
                 shrinkFactor = shrinkFactor * shrinkFactor; 
                 sat.position *= shrinkFactor;
            }
            break;
    }
}

// Update path based on axis-angle rotation
void SatellitesScene::updateSatellitePath(Satellite& sat, float dt, float chaosSetting) {
    if (dt <= 1e-6f) return; 

    // 1. Update Phase Angle
    sat.phaseAngle += sat.angularSpeed * dt;
    while (sat.phaseAngle >= 2.0f * M_PI) sat.phaseAngle -= 2.0f * M_PI;
    while (sat.phaseAngle < 0.0f) sat.phaseAngle += 2.0f * M_PI;

    // 2. Apply Chaos (Nudge axis, ref vec, speed)
    if (chaosSetting > 0.0f) {
        float chaosEffect = chaosSetting * CHAOS_FORCE_SCALE * dt; 
        if (randomFloat(0.0f, 1.0f) < 0.1f * dt * 50) { 
            // Nudge speed
            sat.angularSpeed *= (1.0f + randomFloat(-0.05f, 0.05f) * chaosEffect);
            // Nudge Axis
            Eigen::Vector3f axisNudge = Eigen::Vector3f(randomFloat(-1,1), randomFloat(-1,1), randomFloat(-1,1)).normalized() * chaosEffect * 0.5f;
            sat.orbitAxis = (sat.orbitAxis + axisNudge).normalized();
            // Re-orthogonalize RefVec
            sat.orbitRefVec = sat.orbitAxis.cross(sat.orbitRefVec).cross(sat.orbitAxis).normalized();
        }
    }

    // 3. Recalculate Cartesian position using Rodrigues' formula components
    float angle = sat.phaseAngle;
    float cosTheta = std::cos(angle);
    float sinTheta = std::sin(angle);
    Eigen::Vector3f k = sat.orbitAxis;
    Eigen::Vector3f v = sat.orbitRefVec;
    Eigen::Vector3f kCrossV = k.cross(v);
    float kDotV = k.dot(v); // Should be near zero if orthogonal

    Eigen::Vector3f rotatedVec = v * cosTheta + kCrossV * sinTheta + k * kDotV * (1.0f - cosTheta);
    sat.position = rotatedVec * BASE_ORBITAL_RADIUS; // Scale to radius
}

void SatellitesScene::applySatelliteInteractions(float chaosSetting) {
    if (satellites.size() <= 1) return;
    const float collisionProximitySq = COLLISION_PROXIMITY * COLLISION_PROXIMITY; 

    for (size_t i = 0; i < satellites.size(); ++i) {
        auto& sat1 = satellites[i];
        // Skip if not in a state to collide
        if (sat1.state == Satellite::State::Dead || sat1.state == Satellite::State::Spawning || sat1.state == Satellite::State::Crashing) continue;
        
        for (size_t j = i + 1; j < satellites.size(); ++j) {
            auto& sat2 = satellites[j];
            if (sat2.state == Satellite::State::Dead || sat2.state == Satellite::State::Spawning || sat2.state == Satellite::State::Crashing) continue;
            
            Eigen::Vector3f diff = sat2.position - sat1.position;
            float distanceSq = diff.squaredNorm();

            if (distanceSq < collisionProximitySq) {
                // Collision detected!
                Eigen::Vector3f impactPoint = sat1.position + diff * 0.5f;
                
                // Create Spark Particles
                int numSparks = random(MIN_SPARKS_PER_COLLISION, MAX_SPARKS_PER_COLLISION + 1);
                float sparkLifetimeBase = randomFloat(MIN_SPARK_LIFETIME, MAX_SPARK_LIFETIME);
                for (int k = 0; k < numSparks; ++k) {
                    SparkParticle spark;
                    spark.position = impactPoint;
                    // Eject sparks: Generate unique random direction for EACH spark
                    Eigen::Vector3f randomDir( // Initialize components directly
                        randomFloat(-1.0f, 1.0f),
                        randomFloat(-1.0f, 1.0f),
                        randomFloat(-1.0f, 1.0f)
                    );
                    // Ensure non-zero vector before normalizing
                     if (randomDir.squaredNorm() < 1e-6f) {
                         randomDir.x() = 1.0f; 
                     }
                     randomDir.normalize(); // Normalize after creation
                     
                    // Set velocity to a slow drift in its unique random direction
                    spark.velocity = randomDir * SPARK_BASE_SPEED;
                    spark.lifetime = sparkLifetimeBase * randomFloat(0.8f, 1.2f); 
                    spark.initialLifetime = spark.lifetime; 
                    spark.color = CRGB::Red;
                    sparkParticles.push_back(spark); 
                }
                
                // --- Reduce Speed on Impact --- 
                const float impactSpeedReductionFactor = 0.3f; // Reduce speed to 30%
                sat1.angularSpeed *= impactSpeedReductionFactor * randomFloat(0.8f, 1.2f); // Add some randomness
                sat2.angularSpeed *= impactSpeedReductionFactor * randomFloat(0.8f, 1.2f);
                // --- End Speed Reduction ---

                // Set both to CRASHING state
                sat1.state = Satellite::State::Crashing;
                sat1.timer = CRASH_DURATION + randomFloat(0.0f, 1.5f);
                sat2.state = Satellite::State::Crashing;
                sat2.timer = CRASH_DURATION + randomFloat(0.0f, 1.5f);
                
                // Break inner loop once sat1 is crashing
                break; 
            }
        }
    }
}

void SatellitesScene::renderSatellites() {
    // BENCHMARK_START("render_sat_setup"); // Keep setup benchmark
    size_t numLeds = ledCount();
    if (numLeds == 0) return;
    float renderAngle = settings["render_radius"];
    const uint8_t MIN_BLEND_AMOUNT_HEAD = 4; 
    const float cosRenderAngle = std::cos(std::min(static_cast<float>(M_PI - 1e-4), renderAngle));
    // BENCHMARK_END(); // End render_sat_setup

    // BENCHMARK_START("render_sat_head"); // Combine head/tail benchmarks
    for (const auto& sat : satellites) {
        if (sat.state == Satellite::State::Dead) continue;
        
        CRGB baseColor;
         switch (sat.state) {
            case Satellite::State::Spawning:  baseColor = CRGB::Grey; break;
            case Satellite::State::Orbiting:  baseColor = CRGB::Green; break;
            case Satellite::State::Crashing:  baseColor = CRGB::OrangeRed; break;
            default:                          baseColor = CRGB::Gray; break; // Should only be Dead, but default needed
        }
        CRGB finalSatColor = baseColor; 

        // State modifications
        if (sat.state == Satellite::State::Crashing) {
             float crashProgress = 1.0f - (sat.timer / CRASH_DURATION);
             finalSatColor = blend(baseColor, CRGB::LightYellow, static_cast<uint8_t>(crashProgress * 255)); // Crash to Grey
        } else if (sat.state == Satellite::State::Spawning) {
             float spawnProgress = 1.0f - (sat.timer / SPAWN_DURATION);
             finalSatColor.nscale8(static_cast<uint8_t>(spawnProgress * 255));
        }

        // Render Head (Angular)
        if (sat.position.squaredNorm() < 1e-6f) continue;
        Eigen::Vector3f satDir = sat.position.normalized();

        for (size_t i = 0; i < numLeds; ++i) {
            const auto& p = model().point(i);
            Eigen::Vector3f ledDir = Eigen::Vector3f(p.x(), p.y(), p.z()).normalized();
            float dot = satDir.dot(ledDir);
            if (dot > cosRenderAngle && dot <= 1.0f) {
                float falloff = 0.0f;
                float denominator = 1.0f - cosRenderAngle;
                if (denominator > 1e-6f) {
                    float t_map = (dot - cosRenderAngle) / denominator;
                    falloff = t_map * t_map; // Quadratic falloff (softer edge)
                } else {
                     falloff = 1.0f;
                }
                falloff = std::max(0.0f, std::min(1.0f, falloff)); 
                uint8_t blendAmount = static_cast<uint8_t>(MIN_BLEND_AMOUNT_HEAD + falloff * (255.0f - MIN_BLEND_AMOUNT_HEAD));
                nblend(leds[i], finalSatColor, blendAmount);
            }
        }
        // Tail rendering removed for simplicity
    } // End satellite loop
    // BENCHMARK_END(); // End render_sat_head
}

void SatellitesScene::initializeSatellite(Satellite& sat) {
    sat.uniqueId = nextUniqueId++;

    // Initialize Path Parameters
    sat.phaseAngle = randomFloat(0.0f, 2.0f * M_PI); 
    sat.angularSpeed = BASE_ANGULAR_SPEED * randomFloat(0.8f, 1.2f) * (random(2) == 0 ? 1.0f : -1.0f);
    
    // Init Orientation Axis/Ref Vector
    sat.orbitAxis = Eigen::Vector3f(randomFloat(-1,1), randomFloat(-1,1), randomFloat(-1,1));
    if (sat.orbitAxis.squaredNorm() < 1e-6) sat.orbitAxis = Eigen::Vector3f::UnitZ();
    sat.orbitAxis.normalize();
    sat.orbitRefVec = sat.orbitAxis.cross(Eigen::Vector3f::UnitZ());
    if (sat.orbitRefVec.squaredNorm() < 1e-6) { 
         sat.orbitRefVec = sat.orbitAxis.cross(Eigen::Vector3f::UnitX());
    }
    sat.orbitRefVec.normalize();
     
    // Calculate initial position using Rodrigues' formula 
    float angle = sat.phaseAngle;
    float cosTheta = std::cos(angle); float sinTheta = std::sin(angle);
    Eigen::Vector3f k = sat.orbitAxis; Eigen::Vector3f v = sat.orbitRefVec;
    Eigen::Vector3f kCrossV = k.cross(v); float kDotV = k.dot(v); // kDotV should be ~0
    Eigen::Vector3f rotatedVec = v * cosTheta + kCrossV * sinTheta + k * kDotV * (1.0f - cosTheta);
    sat.position = rotatedVec * BASE_ORBITAL_RADIUS;

}

std::string SatellitesScene::status() const {
    int orbiting = 0, spawning = 0, crashing = 0, dead = 0;
    for (const auto& sat : satellites) {
        switch (sat.state) {
            case Satellite::State::Orbiting: orbiting++; break;
            case Satellite::State::Spawning: spawning++; break;
            case Satellite::State::Crashing: crashing++; break;
            case Satellite::State::Dead:     dead++; break;
        }
    }
    std::string detailStatus = "No satellites";
    if (!satellites.empty()) {
        int firstActive = -1;
        for(size_t i=0; i < satellites.size(); ++i) {
            if(satellites[i].state != Satellite::State::Dead) {
                firstActive = i;
                break;
            }
        }
        if (firstActive != -1) {
            const auto& sat = satellites[firstActive]; 
            char buffer[200];
            // Report basic state for first ACTIVE satellite
            snprintf(buffer, sizeof(buffer),
                     "Sat%lu [S:%d Ph:%.1f Spd:%.2f]",
                     (unsigned long)sat.uniqueId, // Cast to unsigned long
                     static_cast<int>(sat.state), 
                     sat.phaseAngle,   
                     sat.angularSpeed);
            detailStatus = buffer;
        }
    }
    char buffer[200];
    snprintf(buffer, sizeof(buffer), "Pop:%zu O:%d S:%d C:%d | %s",
             satellites.size(), orbiting, spawning, crashing,
             detailStatus.c_str());
    return std::string(buffer);
}

} // namespace Scenes 