#include "animations/boids.h"
#include "points.h"

Boid::Boid(uint16_t unique_id, float speed_limit) : boid_id(unique_id), max_speed(speed_limit) {
    reset(speed_limit);
}

void Boid::reset(float speed_limit) {
    max_speed = speed_limit;
    
    // Random position on sphere using uniform distribution
    float theta = random(TWO_PI * 1000) / 1000.0f;
    float phi = acos(2.0f * random(1000) / 1000.0f - 1);
    pos = Vector3d(
        sin(phi) * cos(theta),
        sin(phi) * sin(theta),
        cos(phi)
    );
    pos *= sphere_r;
    
    // Random initial velocity tangent to sphere
    Vector3d random_vec(random(-1000, 1000), random(-1000, 1000), random(-1000, 1000));
    vel = random_vec.cross(pos.normalized());
    vel.normalize();
    vel *= max_speed;  // Start at max speed
}

void Boid::getSphericalCoords(float& a, float& c) const {
    Vector3d norm_pos = pos.normalized();
    c = acos(norm_pos.z());
    a = atan2(norm_pos.y(), norm_pos.x());
}

void Boid::applyForce(const Vector3d& force) {
    vel += force;
    limitSpeed();
    constrainToSphere();
}

void Boid::constrainToSphere() {
    // Project velocity to be tangent to sphere at current position
    Vector3d norm_pos = pos.normalized();
    vel -= vel.dot(norm_pos) * norm_pos;
}

void Boid::tick() {
    // Constant velocity tangent to sphere
    if (vel.norm() < 0.001f) {
        // Initialize velocity if it's zero
        Vector3d tangent(-pos.y(), pos.x(), 0);  // Rotate around Z axis
        tangent.normalize();
        vel = tangent * max_speed;
    }
    
    // Update position
    pos += vel;
    pos = pos.normalized() * sphere_r;
    
    // Keep velocity tangent to new position
    constrainToSphere();
    vel = vel.normalized() * max_speed;  // Maintain constant speed
}

void BoidsAnimation::init(const AnimParams& params) {
    // Load parameters
    int num_boids = params.getInt("num_boids", 5);
    visual_range = params.getFloat("visual_range", 0.8f);
    protected_range = params.getFloat("protected_range", 0.2f);
    centering_factor = params.getFloat("centering_factor", 0.01f);
    avoid_factor = params.getFloat("avoid_factor", 0.1f);
    matching_factor = params.getFloat("matching_factor", 0.1f);
    speed_limit = params.getFloat("speed_limit", 0.02f);
    fade_amount = params.getInt("fade", 5);
    
    // Get palette for colors
    const CRGBPalette16 palette = params.getPalette("palette", RainbowColors_p);
    
    // Create boids with unique colors
    boids.clear();
    for (int i = 0; i < num_boids; i++) {
        auto boid = std::make_unique<Boid>(i, speed_limit);
        boid->color = ColorFromPalette(palette, i * (256 / num_boids));
        boids.push_back(std::move(boid));
    }
}

int BoidsAnimation::findClosestLED(float a, float c) const {
    // Convert spherical to cartesian
    int x = Boid::sphere_r * sin(c) * cos(a);
    int y = Boid::sphere_r * sin(c) * sin(a);
    int z = Boid::sphere_r * cos(c);
    
    // Start with first LED as initial guess
    int closest = 0;
    int min_dist_sq = INT_MAX;
    bool improved;
    
    do {
        improved = false;
        
        // Check all neighbors of current closest LED
        for (const auto& neighbor : points[closest].neighbors) {
            int dx = points[neighbor.led_number].x - x;
            int dy = points[neighbor.led_number].y - y;
            int dz = points[neighbor.led_number].z - z;
            int dist_sq = dx*dx + dy*dy + dz*dz;
            
            if (dist_sq < min_dist_sq) {
                min_dist_sq = dist_sq;
                closest = neighbor.led_number;
                improved = true;
                break;  // Found a better neighbor, restart from there
            }
        }
    } while (improved);
    
    return closest;
}

// Add this helper function to test if a point is inside a triangle
bool BoidsAnimation::pointInTriangle(const Vector3d& p, const Vector3d& a, const Vector3d& b, const Vector3d& c) const {
    // Calculate triangle normal
    Vector3d normal = (b - a).cross(c - a);
    normal.normalize();
    
    // Calculate barycentric coordinates
    Vector3d v0 = b - a;
    Vector3d v1 = c - a;
    Vector3d v2 = p - a;
    
    float d00 = v0.dot(v0);
    float d01 = v0.dot(v1);
    float d11 = v1.dot(v1);
    float d20 = v2.dot(v0);
    float d21 = v2.dot(v1);
    
    float denom = d00 * d11 - d01 * d01;
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;
    
    // Point is inside if all coordinates are between 0 and 1
    return (u >= 0) && (v >= 0) && (w >= 0) && (u + v + w <= 1.1f);  // Small epsilon for floating point
}

void BoidsAnimation::drawBoid(const Boid& boid) {
    // First find the closest LED to the boid
    int center_led = 0;
    float min_dist = 1e9;
    for (int i = 0; i < numLeds(); i++) {
        Vector3d led_pos(points[i].x, points[i].y, points[i].z);
        float dist = (led_pos - boid.pos).norm();
        if (dist < min_dist) {
            min_dist = dist;
            center_led = i;
        }
    }
    
    // Now spread out from center LED using neighbor distances
    std::set<int> visited;
    std::queue<std::pair<int, float>> to_visit;
    to_visit.push({center_led, 0});
    
    while (!to_visit.empty()) {
        auto [led, dist] = to_visit.front();
        to_visit.pop();
        
        if (visited.find(led) != visited.end()) continue;
        visited.insert(led);
        
        // Light this LED based on network distance from center
        if (dist < 25) {  // Reduced from 60 to 35
            // Steeper falloff curve using square of distance
            float falloff = 1.0f - (dist * dist) / (35.0f * 35.0f);
            uint8_t brightness = 255 * falloff;
            
            CRGB new_color = boid.color;
            new_color.nscale8(brightness);
            nblend(leds[led], new_color, 192);
            
            // Add neighbors with accumulated distances
            for (const auto& neighbor : points[led].neighbors) {
                if (visited.find(neighbor.led_number) == visited.end()) {
                    float new_dist = dist + neighbor.distance;
                    to_visit.push({neighbor.led_number, new_dist});
                }
            }
        }
    }
}

void Boid::limitSpeed() {
    float speed = vel.norm();
    if (speed > max_speed) {
        vel *= max_speed / speed;
    }
}

void BoidsAnimation::tick() {
    // Just update and draw boids, no flocking behavior
    for (auto& boid : boids) {
        boid->tick();
        drawBoid(*boid);
    }
    
    fadeToBlackBy(leds, numLeds(), fade_amount);
}

String BoidsAnimation::getStatus() const {
    output.printf("Boids: %d active (speed=%.2f, fade=%d)\n", 
        (int)boids.size(), speed_limit, fade_amount);
    output.printf("Ranges: Visual %.2f  Protected %.2f\n", 
        visual_range, protected_range);
    
    for (const auto& boid : boids) {
        float a, c;
        boid->getSphericalCoords(a, c);
        output.print(getAnsiColorString(boid->color));
        output.printf(" Boid %d: pos(%.1f, %.1f)\n",
            boid->boid_id, 
            a * 180/PI, c * 180/PI);
    }
    return output.get();
}

float BoidsAnimation::sphericalDistance(const Boid& b1, const Boid& b2) const {
    return (b1.pos - b2.pos).norm();
} 