# Sparkles Scene

## Description

This scene creates a shimmering, sparkling effect by blending colors from two distinct palettes and randomly lighting up pixels across the model. The colors, blend mix, and sparkle intensity evolve over time.

## Core Concept

The Sparkles scene operates based on the following principles:

1.  **Dual Palettes:** It uses two color palettes (e.g., `CloudColors` and `HeatColors`) as the source for its colors.
2.  **Oscillating Colors:** Two indices track positions within these palettes. Each index oscillates around the midpoint (128) within a range defined by `color_range`. The speed of this oscillation is controlled by `color_change_speed`. By default, a full oscillation cycle takes approximately 10 seconds. Each index has its own position and speed tracking.
3.  **Color Blending:** A `color_blend` variable oscillates between 0 and 255 at a rate determined by `blend_cycle_speed`. This variable controls the mixture or dominance of colors from the two palettes.
4.  **Pixel Sparkles:** In each frame, pixels are randomly selected in batches to be illuminated. The `density` parameter influences how frequently pixels are chosen. Newly lit pixels are blended onto the existing color.
5.  **Blend Influence:** The `color_blend` value, along with `density`, affects the *probability* of a sparkle originating from either palette 1 or palette 2. When `color_blend` is near 0 or 255, sparkles predominantly come from one palette.
6.  **Global Fade:** All LEDs fade slightly each frame. The amount of fading is inversely related to `density` – higher density means less fading, lower density means more fading.
7.  **Chaos Influence:** The `chaos` parameter introduces smooth, random variations to the speeds of the color index oscillations and the `color_blend` oscillation. It acts as a scaling factor for a slowly wandering offset applied to these speeds, ensuring gradual, unpredictable changes rather than abrupt jumps.

## Parameters

*   `blend_cycle_speed`: Controls how quickly the `color_blend` variable oscillates between favoring palette 1 and palette 2.
*   `color_change_speed`: Controls the base speed at which the two palette indices oscillate.
*   `color_range`: Defines the amplitude of the palette index oscillation around the midpoint (128).
*   `density`: Influences the frequency of sparkle generation (more density = more sparkles) and the rate of global fade (more density = less fade).
*   `chaos`: Controls the magnitude of smooth, random variations applied to the color index and blend oscillation speeds.

## Implementation Notes

*   The oscillation logic uses separate position and velocity variables for each palette index.
*   The chaos effect is implemented by adding a slowly changing random offset (scaled by the `chaos` parameter) to the base speeds.
*   Pixel selection involves random number generation, scaled by `density`.
*   Color assignment probability is weighted by the current `color_blend` value. 