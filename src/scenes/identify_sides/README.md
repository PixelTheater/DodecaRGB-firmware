# Identify Sides Scene

## Description

This scene is designed to help with alignment and configuration of the DodecaRGB device by displaying all faces and connected edges with their unique colors while cycling which color pulses. This allows the user to visually verify the wiring and configuration of the model, by making appropriate changes to the YAML file and re-generating the model.h file and re-testing.

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
   - **Center Face Identification**:
     - For each face: Light first N LEDs with face color where N = geometric_position + 1
     - This creates a unique LED count pattern for each face (pulse if matching current color)
   - **Safe Edge Detection**:
     - For each face and each of its edges:
       - Find the adjacent face using edge adjacency data
       - Calculate distances from current face LEDs to all adjacent face LEDs
       - Find LEDs on the current face closest to the adjacent face
       - Light the closest N LEDs where N = adjacent_face_id + 1 in the adjacent face's color
       - This creates edge patterns that show both connectivity AND face identity through LED count
     - Apply brightness scaling and pulsing to all colors

## Usage

This scene is particularly useful for visually verifying the wiring and configuration of a model. The faces are the same but the wiring and order may be different. This helps us ensure the model is configured correctly so the logical faces and pixels are arranged in a consistent way, regardless of how the model is wired.

## Visual Pattern

The animation cycles through faces 0-11, spending 3 seconds on each, slowly pulsing both the center cluster and all edge clusters that reference that face. This results in each of the 12 colors being animated, helping to identify problems with face remapping and rotations.

**Key Features**:
- Each face shows a unique color (evenly distributed HSV hues)
- **Center clusters**: Each face shows N LEDs where N = geometric_position + 1 for easy identification
- **Edge clusters**: Each edge shows N LEDs where N = connected_face_id + 1 in the connected face's color
- Only the current color pulses at 60 BPM (adjustable with Speed parameter)
- **Dual identification**: Both center and edge patterns use LED count to identify face numbers
- Simultaneous view of all face and edge relationships using pure geometry
- **Stable and safe**: Uses only IModel interface methods to avoid crashes with different model types
- If edge clusters show unexpected colors or counts, it indicates model configuration issues

**Diagnostic Information**:
- Center cluster count = geometric position + 1
- Edge cluster count = connected face ID + 1  
- Edge cluster color = connected face color
- This allows verification of both geometric positioning and logical connectivity

## Notes

- **Safe interface usage**: Uses only the stable IModel interface methods to avoid crashes
- **Distance-based LED selection**: Finds LEDs closest to adjacent faces rather than using predefined LED groups
- **Portable implementation**: Works with any model type without requiring specific model knowledge
- **Geometric awareness**: Automatically handles face remapping through the model's geometric positioning system
- Performance scales with LED count per face but should be acceptable for typical models
- More reliable than LED group-based approaches for diagnostic purposes
