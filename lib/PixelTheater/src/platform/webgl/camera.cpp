#include "PixelTheater/platform/webgl/camera.h"
#include "PixelTheater/platform/webgl/math.h"
#include <cmath>
#include <algorithm>

namespace PixelTheater {

// Distance presets in world units
static constexpr float DISTANCE_CLOSE = 10.0f;
static constexpr float DISTANCE_NORMAL = 20.0f;
static constexpr float DISTANCE_FAR = 30.0f;

// View angles
static constexpr float TOP_VIEW_ANGLE = static_cast<float>(M_PI / 2.0f);  // 90 degrees
static constexpr float SIDE_VIEW_ANGLE = 0.0f;                            // 0 degrees
static constexpr float ANGLE_VIEW_ANGLE = static_cast<float>(M_PI / 4.0f); // 45 degrees

Camera::Camera() 
    : _cameraHeight(0.0f),      // Start at eye level
      _cameraDistance(20.0f),   // Start at medium distance
      _modelRotationX(0.0f),    // X-axis rotation (pitch)
      _modelRotationY(0.0f),    // Y-axis rotation (yaw)
      _modelRotationZ(0.0f),    // Z-axis rotation (roll)
      _autoRotate(true),        // Start with auto-rotation
      _autoRotationSpeed(SLOW_ROTATION_SPEED),  // Start with slow rotation
      _viewAngle(ANGLE_VIEW_ANGLE)  // Start at 45-degree view
{
    // Set initial camera position to a good viewing angle
    setPresetView(ViewPreset::ANGLE, DistancePreset::NORMAL);
}

void Camera::setHeight(float height) {
    _cameraHeight = height;
}

void Camera::setDistance(float distance) {
    _cameraDistance = std::max(1.0f, distance);
}

void Camera::setCameraDistance(float distance) {
    setDistance(distance);  // Alias for compatibility
}

float Camera::calculateTilt() const {
    return _viewAngle;  // View angle determines tilt directly
}

void Camera::updateModelRotation(float deltaX, float deltaY) {
    // Convert mouse movement to rotation angles
    _modelRotationY += deltaX;
    _modelRotationX += deltaY;
    
    // Clamp pitch to prevent over-rotation
    const float maxPitch = static_cast<float>(M_PI) / 2.0f;
    _modelRotationX = std::max(-maxPitch, std::min(maxPitch, _modelRotationX));
    
    // Keep yaw between 0 and 2π
    while (_modelRotationY < 0.0f) {
        _modelRotationY += static_cast<float>(M_PI * 2.0f);
    }
    while (_modelRotationY >= static_cast<float>(M_PI * 2.0f)) {
        _modelRotationY -= static_cast<float>(M_PI * 2.0f);
    }
}

void Camera::resetModelRotation() {
    _modelRotationX = 0.0f;
    _modelRotationY = 0.0f;
    _modelRotationZ = 0.0f;
}

void Camera::resetRotation() {
    resetModelRotation();  // Alias for compatibility
}

void Camera::setPresetView(ViewPreset view) {
    // Use current distance when not specified
    DistancePreset currentDistance;
    if (_cameraDistance <= DISTANCE_CLOSE) {
        currentDistance = DistancePreset::CLOSE;
    } else if (_cameraDistance <= DISTANCE_NORMAL) {
        currentDistance = DistancePreset::NORMAL;
    } else {
        currentDistance = DistancePreset::FAR;
    }
    setPresetView(view, currentDistance);
}

void Camera::setPresetView(ViewPreset view, DistancePreset distance) {
    // Stop auto-rotation when setting a specific view
    _autoRotate = false;
    
    // Set camera position based on view and distance presets
    setCameraPosition(view, distance);
}

void Camera::setCameraPosition(ViewPreset view, DistancePreset distance) {
    // Set distance based on preset
    switch (distance) {
        case DistancePreset::CLOSE:
            _cameraDistance = DISTANCE_CLOSE;
            break;
        case DistancePreset::NORMAL:
            _cameraDistance = DISTANCE_NORMAL;
            break;
        case DistancePreset::FAR:
            _cameraDistance = DISTANCE_FAR;
            break;
    }
    
    // Set view angle based on preset
    switch (view) {
        case ViewPreset::SIDE:
            _viewAngle = SIDE_VIEW_ANGLE;  // 0 degrees - horizontal view
            break;
            
        case ViewPreset::ANGLE:
            _viewAngle = ANGLE_VIEW_ANGLE;  // 45 degrees
            break;
            
        case ViewPreset::TOP:
            _viewAngle = TOP_VIEW_ANGLE;    // 90 degrees - directly overhead
            break;
    }
    
    // Calculate height based on view angle
    _cameraHeight = _cameraDistance * std::sin(_viewAngle);
}

void Camera::updateAutoRotation(float deltaTime) {
    if (_autoRotate) {
        // Only rotate around world Y axis for turntable effect
        // Ensure deltaTime is reasonable to prevent jumps
        float cappedDelta = std::min(deltaTime, 0.1f);
        _modelRotationY += _autoRotationSpeed * cappedDelta;
        
        // Keep angle in reasonable range
        while (_modelRotationY > 2.0f * M_PI) {
            _modelRotationY -= 2.0f * M_PI;
        }
    }
}

void Camera::toggleAutoRotation() {
    _autoRotate = !_autoRotate;
}

void Camera::calculateViewMatrix(float* matrix) {
    // Initialize matrix to identity
    for (int i = 0; i < 16; i++) {
        matrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
    
    // Calculate camera position in world space
    float tilt = _viewAngle;  // Use view angle directly
    float cosTilt = std::cos(tilt);
    float sinTilt = std::sin(tilt);
    
    // Calculate camera position in world space
    // For top view (90 degrees), cosine will be 0 and sine will be 1
    float eyeX = 0.0f;
    float eyeY = _cameraDistance * sinTilt;  // Will be _cameraDistance for top view
    float eyeZ = -_cameraDistance * cosTilt; // Will be 0 for top view
    
    // Center position (where the model is)
    float centerX = 0.0f;
    float centerY = 0.0f;
    float centerZ = 0.0f;
    
    // Up vector (always world up for turntable effect)
    float upX = 0.0f;
    float upY = 1.0f;
    float upZ = 0.0f;
    
    // Calculate forward vector (direction the camera is looking)
    float forwardX = centerX - eyeX;
    float forwardY = centerY - eyeY;
    float forwardZ = centerZ - eyeZ;
    
    // Normalize forward vector
    float forwardLength = std::sqrt(forwardX * forwardX + forwardY * forwardY + forwardZ * forwardZ);
    if (forwardLength > 0.0001f) {  // Avoid division by zero
        forwardX /= forwardLength;
        forwardY /= forwardLength;
        forwardZ /= forwardLength;
    }
    
    // For top view, ensure right vector is along world X axis
    float rightX, rightY, rightZ;
    if (std::abs(tilt - TOP_VIEW_ANGLE) < 0.001f) {
        // When looking straight down, right is along world X
        rightX = 1.0f;
        rightY = 0.0f;
        rightZ = 0.0f;
    } else {
        // Calculate right vector as cross product of forward and world up
        rightX = forwardZ;
        rightY = 0.0f;
        rightZ = -forwardX;
        
        // Normalize right vector
        float rightLength = std::sqrt(rightX * rightX + rightZ * rightZ);
        if (rightLength > 0.0001f) {
            rightX /= rightLength;
            rightZ /= rightLength;
        }
    }
    
    // Calculate camera up vector as cross product of right and forward
    float trueUpX = rightY * forwardZ - rightZ * forwardY;
    float trueUpY = rightZ * forwardX - rightX * forwardZ;
    float trueUpZ = rightX * forwardY - rightY * forwardX;
    
    // Build view matrix (camera only)
    matrix[0] = rightX;
    matrix[1] = rightY;
    matrix[2] = rightZ;
    matrix[3] = 0.0f;
    
    matrix[4] = trueUpX;
    matrix[5] = trueUpY;
    matrix[6] = trueUpZ;
    matrix[7] = 0.0f;
    
    matrix[8] = -forwardX;
    matrix[9] = -forwardY;
    matrix[10] = -forwardZ;
    matrix[11] = 0.0f;
    
    matrix[12] = -(rightX * eyeX + rightY * eyeY + rightZ * eyeZ);
    matrix[13] = -(trueUpX * eyeX + trueUpY * eyeY + trueUpZ * eyeZ);
    matrix[14] = -(-forwardX * eyeX + -forwardY * eyeY + -forwardZ * eyeZ);
    matrix[15] = 1.0f;
}

void Camera::getModelRotationMatrix(float* matrix) {
    // Initialize to identity
    for (int i = 0; i < 16; i++) {
        matrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
    
    // For turntable effect, we only rotate around world Y axis
    float cosY = std::cos(_modelRotationY);
    float sinY = std::sin(_modelRotationY);
    
    // Y rotation (turntable)
    matrix[0] = cosY;
    matrix[2] = sinY;
    matrix[8] = -sinY;
    matrix[10] = cosY;
}

} // namespace PixelTheater 