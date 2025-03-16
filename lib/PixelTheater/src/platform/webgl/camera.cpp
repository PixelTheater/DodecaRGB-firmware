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

// Camera vertical offset to center the model in the viewport
static constexpr float CAMERA_Y_OFFSET = 5.0f;

Camera::Camera() 
    : _cameraHeight(0.0f),      // Start at eye level
      _cameraDistance(20.0f),   // Start at medium distance
      _modelRotationX(0.0f),    // X-axis rotation (pitch)
      _modelRotationY(0.0f),    // Y-axis rotation (yaw)
      _modelRotationZ(0.0f),    // Z-axis rotation (roll)
      _autoRotate(true),        // Start with auto-rotation
      _autoRotationSpeed(SLOW_ROTATION_SPEED),  // Start with slow rotation
      _viewAngle(SIDE_VIEW_ANGLE)  // Start with side view (0 degrees)
{
    // Set initial camera position to side view
    setPresetView(ViewPreset::SIDE, DistancePreset::NORMAL);
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
    // Negate deltaX to reverse the direction for left-right rotation
    _modelRotationY -= deltaX;
    
    // For up-down rotation, we need to consider the current orientation
    // to ensure consistent behavior regardless of model orientation
    
    // Calculate the effective rotation direction based on current Y rotation
    // This ensures that "up" always means "up" in screen space
    float cosY = std::cos(_modelRotationY);
    float sinY = std::sin(_modelRotationY);
    
    // Apply deltaY with direction correction based on current orientation
    // When model is rotated 180 degrees, we need to reverse the direction
    _modelRotationX -= deltaY * (cosY > 0 ? 1.0f : -1.0f);
    
    // Keep X rotation (pitch) between 0 and 2π, allowing full rotation
    while (_modelRotationX < 0.0f) {
        _modelRotationX += static_cast<float>(M_PI * 2.0f);
    }
    while (_modelRotationX >= static_cast<float>(M_PI * 2.0f)) {
        _modelRotationX -= static_cast<float>(M_PI * 2.0f);
    }
    
    // Keep Y rotation (yaw) between 0 and 2π
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
    
    // Calculate camera position in world space based on view angle
    float tilt = _viewAngle;
    float cosTilt = std::cos(tilt);
    float sinTilt = std::sin(tilt);
    
    // Calculate camera position in world space
    float eyeX = 0.0f;
    float eyeY = _cameraDistance * sinTilt;
    float eyeZ = _cameraDistance * cosTilt;
    
    // The model's center is at (0,0,0) in world space
    float centerX = 0.0f;
    float centerY = 0.0f;
    float centerZ = 0.0f;
    
    // Calculate forward vector (direction the camera is looking)
    float forwardX = centerX - eyeX;
    float forwardY = centerY - eyeY;
    float forwardZ = centerZ - eyeZ;
    
    // Normalize forward vector
    float forwardLength = std::sqrt(forwardX * forwardX + forwardY * forwardY + forwardZ * forwardZ);
    if (forwardLength > 0.0001f) {
        forwardX /= forwardLength;
        forwardY /= forwardLength;
        forwardZ /= forwardLength;
    }
    
    // World up vector and right vector calculation
    float upX, upY, upZ;
    float rightX, rightY, rightZ;
    
    // Check if we're in top view (or very close to it)
    bool isTopView = std::abs(tilt - TOP_VIEW_ANGLE) < 0.01f;
    
    if (isTopView) {
        // For top view, use Z axis as the up vector to avoid singularity
        // This ensures the camera's right vector is along the X axis
        upX = 0.0f;
        upY = 0.0f;
        upZ = 1.0f;
        
        // Right vector is perpendicular to forward and up
        rightX = 1.0f;  // Align with world X axis
        rightY = 0.0f;
        rightZ = 0.0f;
    } else {
        // Standard case - use world Y as up vector
        upX = 0.0f;
        upY = 1.0f;
        upZ = 0.0f;
        
        // Calculate right vector as cross product of forward and world up
        rightX = forwardY * upZ - forwardZ * upY;
        rightY = forwardZ * upX - forwardX * upZ;
        rightZ = forwardX * upY - forwardY * upX;
        
        // Normalize right vector
        float rightLength = std::sqrt(rightX * rightX + rightY * rightY + rightZ * rightZ);
        if (rightLength > 0.0001f) {
            rightX /= rightLength;
            rightY /= rightLength;
            rightZ /= rightLength;
        }
    }
    
    // Calculate camera up vector as cross product of right and forward
    float trueUpX = rightY * forwardZ - rightZ * forwardY;
    float trueUpY = rightZ * forwardX - rightX * forwardZ;
    float trueUpZ = rightX * forwardY - rightY * forwardX;
    
    // Build view matrix
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
    
    // Translation
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
    
    // Apply Y rotation (around Y axis) for turntable effect
    float cosY = std::cos(_modelRotationY);
    float sinY = std::sin(_modelRotationY);
    
    // Standard Y-axis rotation matrix
    matrix[0] = cosY;
    matrix[2] = -sinY;
    matrix[8] = sinY;
    matrix[10] = cosY;
    
    // Apply X rotation (around X axis) if needed
    if (_modelRotationX != 0.0f) {
        float cosX = std::cos(_modelRotationX);
        float sinX = std::sin(_modelRotationX);
        
        // We need a temporary matrix for this
        float tempMatrix[16];
        for (int i = 0; i < 16; i++) {
            tempMatrix[i] = matrix[i];
        }
        
        // Apply X rotation to the Y rotation
        matrix[4] = tempMatrix[4] * cosX + tempMatrix[8] * sinX;
        matrix[5] = tempMatrix[5] * cosX + tempMatrix[9] * sinX;
        matrix[6] = tempMatrix[6] * cosX + tempMatrix[10] * sinX;
        matrix[7] = tempMatrix[7] * cosX + tempMatrix[11] * sinX;
        
        matrix[8] = tempMatrix[4] * -sinX + tempMatrix[8] * cosX;
        matrix[9] = tempMatrix[5] * -sinX + tempMatrix[9] * cosX;
        matrix[10] = tempMatrix[6] * -sinX + tempMatrix[10] * cosX;
        matrix[11] = tempMatrix[7] * -sinX + tempMatrix[11] * cosX;
    }
}

} // namespace PixelTheater 