# XYZ Scanner Scene

The XYZ Scanner scene creates three scanning planes (red, green, and blue) that move through the model along the X, Y, and Z axes. The planes create interesting visual effects as they intersect and blend.

## Visual Effect

This animation creates three colored scanning planes:
- A **red plane** that moves along the Y axis
- A **green plane** that moves along the X axis
- A **blue plane** that moves along the Z axis

Each plane has a thickness determined by the `target` value, which oscillates over time. When LEDs are within the thickness of a plane, they light up with the corresponding color. The intensity of the color depends on how close the LED is to the center of the plane.

As the planes move through the model and intersect, they create interesting color blends and patterns.

## Parameters

| Parameter | Type | Default | Range | Description |
|-----------|------|---------|-------|-------------|
| `speed` | float | 0.05 | 0.001 - 5.0 | Controls how fast the planes move through the model |
| `blend` | int | 160 | 10 - 255 | Controls how much the colors blend with existing colors |
| `fade` | int | 35 | 1 - 100 | Controls how quickly the colors fade after a plane passes |

## Implementation Details

The animation uses trigonometric functions to create smooth, oscillating movements:
- The Z plane position is updated using a cosine function
- The Y plane position is updated using a tangent function (constrained to avoid extreme values)
- The X plane position is updated using a sine function

The thickness of each plane oscillates over time using a cosine function, creating a pulsing effect.

## Original Implementation

This scene is a port of the original XYZ Scanner animation from the legacy codebase. The original animation was designed to create a visually interesting effect by scanning through the model with planes of light.

## Usage Tips

- Lower `speed` values create a more subtle, gentle scanning effect
- Higher `blend` values create smoother transitions but less distinct planes
- Lower `fade` values create longer trails as the planes move through the model 