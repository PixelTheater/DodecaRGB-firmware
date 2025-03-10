# Wandering Particles Scene

This scene creates a collection of particles that wander around the model, creating matrix-like flowing trails of light.

## Description

Particles move from LED to LED across the model, following paths that approximate movement in 3D space. Each particle has its own velocity, color, and maintains a history of its recent positions. This creates flowing trails of light that fade over time, similar to the digital rain effect from The Matrix, but with colorful hues.

## Implementation Notes

This scene is a faithful recreation of the original WanderingParticles animation from the legacy codebase, with the following key characteristics:

1. **Slow Movement**: Particles move very slowly through 3D space (using tiny angle increments)
2. **Short Hold Times**: Particles stay at each LED for only 6-12 frames before moving
3. **Limited Neighbor Search**: Only the 7 closest LEDs are considered as potential next positions
4. **Brightness Calculation**: The brightness of each particle is inversely proportional to its age
5. **Trail Rendering**: Both the current position and previous positions are rendered with decreasing brightness

### Neighbor Search Implementation

While the PixelTheater framework defines neighbor relationships in the model definition files, there isn't a direct public API to access this information through the Point or Model classes. As a result, this scene implements a custom neighbor search that:

1. Limits the search to LEDs within a 30mm radius of the current LED
2. Considers at most 7 candidate LEDs (matching the original implementation)
3. Uses the Point::distanceTo method when available for distance calculations
4. Avoids backtracking by preventing particles from revisiting recent LEDs

This approach closely mimics the behavior of the original implementation while being efficient for large models.

## Parameters

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| num_particles | count | 5-200 | 80 | Number of particles |
| fade_amount | count | 1-50 | 20 | Fade amount per frame (higher = faster fade) |
| blend_amount | range | 10-200 | 80 | Blend intensity (higher = brighter particles) |
| reset_chance | count | 1-20 | 2 | Chance of particle reset (higher = more frequent) |

## Visual Effect

The scene creates a subtle, flowing animation with particles that wander slowly around the model. Each particle leaves a trail of illuminated LEDs behind it, creating the impression of digital rain or flowing data streams. The trails fade over time, with the most recent positions being brightest.

## Performance Considerations

- Higher particle counts will increase CPU usage
- Each particle maintains a history of its recent positions, which increases memory usage
- The scene uses a spatial search radius to find nearby LEDs, which is more efficient than searching all LEDs
- The scene avoids backtracking by preventing particles from revisiting recent LEDs 