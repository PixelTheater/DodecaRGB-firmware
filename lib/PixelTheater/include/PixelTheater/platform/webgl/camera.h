#pragma once

namespace PixelTheater {

// Camera class for managing view transformations
// Camera is fixed in position (like on a tripod) while the model rotates
// Camera always points at the model (origin), so tilt is calculated from height and distance
class Camera {
public:
    // Camera view presets - these define fixed camera positions
    enum class ViewPreset {
        SIDE,    // Side view - camera at same Z as model
        ANGLE,   // 45-degree angle view - camera at 45° from horizontal
        TOP      // Top-down view - camera directly above model
    };
    
    // Camera distance presets
    enum class DistancePreset {
        CLOSE,   // Close to model
        NORMAL,  // Medium distance
        FAR      // Far from model
    };
    
    // Auto-rotation speeds (in radians/second)
    static constexpr float SLOW_ROTATION_SPEED = 0.146f;  // 1.4 RPM
    static constexpr float FAST_ROTATION_SPEED = 1.047f;  // 10 RPM
    
    // Constructor
    Camera();
    
    // Camera position methods
    void setHeight(float height);  // Set camera height (tripod height)
    void setDistance(float distance);  // Set camera distance (dolly position)
    void setCameraDistance(float distance);  // Alias for setDistance for compatibility
    
    // Model rotation methods
    void updateModelRotation(float deltaX, float deltaY);  // Free rotation based on mouse movement
    void resetModelRotation();  // Reset all rotations to 0
    void resetRotation();  // Alias for resetModelRotation for compatibility
    
    // Preset view methods
    void setPresetView(ViewPreset view, DistancePreset distance);
    void setPresetView(ViewPreset view);  // Overload that uses current distance
    
    // Auto-rotation methods (rotates the model around Y axis only)
    void updateAutoRotation(float deltaTime);
    void toggleAutoRotation();
    
    // View matrix calculation
    void calculateViewMatrix(float* matrix);
    void getModelRotationMatrix(float* matrix);  // Get model rotation matrix separately
    
    // Getters for current values
    float getHeight() const { return _cameraHeight; }
    float getDistance() const { return _cameraDistance; }
    float getModelRotationX() const { return _modelRotationX; }
    float getModelRotationY() const { return _modelRotationY; }
    float getModelRotationZ() const { return _modelRotationZ; }
    
    // Get/set auto-rotation state
    bool isAutoRotating() const { return _autoRotate; }
    void setAutoRotation(bool autoRotate) { _autoRotate = autoRotate; }
    void setAutoRotationSpeed(float speed) { _autoRotationSpeed = speed; }
    float getAutoRotationSpeed() const { return _autoRotationSpeed; }
    
private:
    // Camera position (fixed)
    float _cameraHeight;    // Height of camera (tripod height)
    float _cameraDistance;  // Distance from model (dolly position)
    
    // Model rotation (changes)
    float _modelRotationX;  // X-axis rotation (pitch)
    float _modelRotationY;  // Y-axis rotation (yaw)
    float _modelRotationZ;  // Z-axis rotation (roll)
    
    // Auto-rotation state
    bool _autoRotate;
    float _autoRotationSpeed;
    
    // View angle (in radians)
    float _viewAngle;  // Current view angle (0 = side, π/2 = top)
    
    // Helper method to calculate tilt angle based on height and distance
    float calculateTilt() const;
    
    // Helper method to set camera position based on view and distance presets
    void setCameraPosition(ViewPreset view, DistancePreset distance);
};

} // namespace PixelTheater 