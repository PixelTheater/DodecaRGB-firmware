#include "hardware/animation_hardware.h"
#include "points.h"

Boid::Boid(uint16_t unique_id, float speed_limit) : boid_id(unique_id), max_speed(speed_limit) {
    reset(speed_limit);
}

void Boid::reset(float speed_limit) {
    max_speed = speed_limit;
    state = State::FOLLOWING;
    setRandomTimer();
    
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

void Boid::setRandomTimer() {
    if (state == State::FOLLOWING) {
        state_timer = millis() + random(MIN_FOLLOW_TIME, MAX_FOLLOW_TIME);
    } else {
        state_timer = millis() + random(MIN_REST_TIME, MAX_REST_TIME);
        // Set first heading change timer
        heading_change_timer = millis() + random(MIN_HEADING_TIME, MAX_HEADING_TIME);
    }
}

void Boid::updateState() {
    uint32_t now = millis();
    
    // Check for state changes
    if (now > state_timer) {
        if (state == State::FOLLOWING) {
            // Use chaos_factor to determine if we should explore
            if (random(1000) < chaos_factor * 1000) {
                state = State::EXPLORING;
                // Give initial push in random direction when starting to explore
                float angle = random(-120, 120) * PI / 180.0f;
                Vector3d up = pos.normalized();
                Vector3d right = vel.cross(up).normalized();
                Vector3d forward = up.cross(right);
                
                Matrix3d rot;
                float c = cos(angle);
                float s = sin(angle);
                rot << c, -s, 0,
                       s,  c, 0,
                       0,  0, 1;
                
                vel = (rot * forward).normalized() * (max_speed * 1.2f);
            } else {
                // Stay following but reset timer
                setRandomTimer();
            }
        } else {
            state = State::FOLLOWING;
            vel *= 0.8f;
        }
        setRandomTimer();
    }
    
    // When exploring, make more dramatic heading changes
    if (state == State::EXPLORING && now > heading_change_timer) {
        float angle = random(-90, 90) * PI / 180.0f;  // ±90 degrees
        Vector3d up = pos.normalized();
        Vector3d right = vel.cross(up).normalized();
        Vector3d forward = up.cross(right);
        
        Matrix3d rot;
        float c = cos(angle);
        float s = sin(angle);
        rot << c, -s, 0,
               s,  c, 0,
               0,  0, 1;
        
        vel = (rot * forward).normalized() * (max_speed * 1.2f);  // Maintain higher speed
        
        heading_change_timer = now + random(MIN_HEADING_TIME, MAX_HEADING_TIME);
    }
}

void Boid::getSphericalCoords(float& a, float& c) const {
    Vector3d norm_pos = pos.normalized();
    c = acos(norm_pos.z());
    a = atan2(norm_pos.y(), norm_pos.x());
}

void Boid::limitSpeed() {
    float speed = vel.norm();
    if (speed > max_speed) {
        vel *= max_speed / speed;
    }
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
    updateState();  // Check if we need to change states
    
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

class BoidsAnimation : public Animation {
public:
    void init(const AnimParams& params) {
        // Load parameters
        int num_boids = params.getInt("num_boids", 5);
        visual_range = params.getFloat("visual_range", 0.8f);
        protected_range = params.getFloat("protected_range", 0.2f);
        centering_factor = params.getFloat("centering_factor", 0.01f);
        avoid_factor = params.getFloat("avoid_factor", 0.1f);
        matching_factor = params.getFloat("matching_factor", 0.1f);
        speed_limit = params.getFloat("speed_limit", 0.02f);
        fade_amount = params.getInt("fade", 5);
        chaos_factor = params.getFloat("chaos", 0.3f);
        boid_size = params.getInt("size", 25);
        intensity = params.getFloat("intensity", 0.8f);
        
        // Get palette for colors
        const CRGBPalette16 palette = params.getPalette("palette", basePalette);
        
        // Create boids with unique colors
        boids.clear();
        for (int i = 0; i < num_boids; i++) {
            auto boid = std::make_unique<Boid>(i, speed_limit);
            boid->chaos_factor = chaos_factor;  // Pass chaos factor to boid
            boid->color = ColorFromPalette(palette, i * (256 / num_boids));
            boids.push_back(std::move(boid));
        }
    }

    int findClosestLED(float a, float c) const {
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
    bool pointInTriangle(const Vector3d& p, const Vector3d& a, const Vector3d& b, const Vector3d& c) const {
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

    struct BoidCone {
        Vector3d pos_dir;     // Normalized position vector
        Vector3d vel_dir;     // Normalized velocity vector
        float cos_angle;      // Cosine of cone angle
        CRGB color;
        float intensity;
    };

    void tick() override {
        // Work with display abstraction
        for (size_t i = 0; i < display().size(); i++) {
            const Point& pos = display().getPoint(i);
            CRGB color = calculateColor(pos);
            display().setPixel(i, color);
        }
    }

    String status() const override {
        output.printf("Boids: %d active (speed=%.2f, fade=%d)\n", 
            (int)boids.size(), speed_limit, fade_amount);
        output.printf("Ranges: Visual %.2f  Protected %.2f\n", 
            visual_range, protected_range);
        
        int following = 0, exploring = 0;
        for (const auto& boid : boids) {
            float a, c;
            boid->getSphericalCoords(a, c);
            char status_char = boid->state == Boid::State::FOLLOWING ? '^' : '?';
            output.print(getAnsiColorString(boid->color, status_char));
            if (boid->state == Boid::State::FOLLOWING) following++; else exploring++;
        }
        output.println("");
        output.printf("States: %d Following, %d Exploring\n", following, exploring);
        return output.get();
    }

    float sphericalDistance(const Boid& b1, const Boid& b2) const {
        // Should be great circle distance:
        Vector3d dir1 = b1.pos.normalized();
        Vector3d dir2 = b2.pos.normalized();
        return acos(dir1.dot(dir2));
    }
}; 