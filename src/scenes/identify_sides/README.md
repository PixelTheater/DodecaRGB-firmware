# Identify Sides Scene

## Description

This scene is designed to help with alignment and configuration of the DodecaRGB device by displaying all faces with their unique colors while cycling which color pulses. Each face lights the first N LEDs where N equals the face ID plus 1 (face 0 = 1 LED, face 1 = 2 LEDs, etc.). All faces and edges are always lit, but only one color pulses at a time, making it easy to identify specific faces and verify edge adjacency.

## Implementation Strategy

The scene creates a color-cycling animation that helps users:
- Identify specific faces by counting LEDs (face 0 has 1 LED, face 1 has 2 LEDs, etc.)
- Identify faces that are wired out of order
- Verify edge adjacency and neighboring face relationships  
- See all face and edge mappings simultaneously
- Confirm proper geometric positioning through color relationships

## Parameters

- **Speed**: (Ratio 0.0-2.0, Default: 1.0) Controls pulse rate. 1.0 = 60 BPM, 2.0 = 120 BPM, 0.0 = no pulsing.
- **Brightness**: (Ratio 0.1-1.0, Default: 0.8) Controls overall brightness of the display. Lower values reduce power consumption and eye strain during setup.

## Algorithm

1. **Initialization (`setup`)**:
   - Define metadata (Name, Author, Description, Version)
   - Define parameters (Speed, Brightness) with appropriate ranges and defaults
   - Log setup completion

2. **Update Loop (`tick`)**:
   - Read current parameter values (Speed, Brightness)
   - Clear all LEDs to black
   - Calculate current pulsing color (3 seconds per color)
   - Calculate 60 BPM pulse factor based on time and speed parameter
   - For all faces:
     - Light first N LEDs with face color where N = face_id + 1 (pulse if matching current color)
   - For all edges:
     - Look up adjacent face using edge adjacency data
     - Light edge LEDs with adjacent face color (pulse if matching current color)
     - Apply brightness scaling to all colors

## Usage

This scene is particularly useful for:
- **Initial Setup**: Verifying that all faces are properly wired and numbered
- **Debugging**: Identifying which physical face corresponds to which face ID in software
- **Alignment**: Ensuring the device orientation matches the expected coordinate system
- **Verification**: Confirming that LED mapping is correct across all faces

## Visual Pattern

The animation cycles through colors 0-11, spending 3 seconds on each:

- **T=0-3s**: All red elements (face centers and edges) pulse, others steady
- **T=3-6s**: All orange-red elements pulse, others steady  
- **T=6-9s**: All orange elements pulse, others steady
- **T=9-12s**: All yellow elements pulse, others steady
- ...continues through all 12 colors, then repeats

**Key Features**:
- Each face lights N LEDs where N = face_id + 1 (face 0: 1 LED, face 1: 2 LEDs, face 2: 3 LEDs, etc.)
- All face LEDs show their unique colors (evenly distributed HSV hues)
- All edges always show their adjacent face colors
- Only the current color pulses at 60 BPM (adjustable with Speed parameter)
- LED count pattern makes it easy to identify specific face numbers
- Simultaneous view of all face and edge relationships
- If edges light up with unexpected colors, it indicates wiring issues

## Notes

- **Edge Adjacency**: The scene uses the model's EDGES array to determine which faces are actually adjacent, lighting edges with the correct neighboring face colors
- **Color Cycling**: Each color pulses for exactly 3 seconds, making it easy to identify all instances of each color across faces and edges
- **Complete Visibility**: All face and edge relationships are visible simultaneously, providing comprehensive validation
- **Pulse Synchronization**: All instances of the current color (centers and edges) pulse in sync at the specified BPM for clear visual feedback
- **Wiring Validation**: If edges show unexpected colors, it indicates faces are wired to wrong positions in the LED chain 