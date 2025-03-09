# Python Utilities

Tools for model generation and development.

## Setup

```bash
pip install -r requirements.txt
```

## Tools

### Model Generation
```bash
# Generate model from YAML definition and PCB pick-and-place data
python util/generate_model.py -d <model_dir>

# Options:
# -m, --model MODEL       Path to model YAML definition file
# -d, --model-dir DIR     Path to model directory containing model.yaml and pcb/*.pos
# -o, --output FILE       Output file (default: model.h in model directory)
# -f, --format FORMAT     Output format: cpp or json (default: cpp)
# -i, --input FILE        Input PCB pick and place file (overrides YAML definition)
# -y, --yes               Automatically overwrite existing files without confirmation

# Example:
python util/generate_model.py -d src/models/DodecaRGBv2
```

### Testing
```bash
python -m util.tests.run_tests
```

### 3D Visualizer
```bash
python visualizer.py
```

Controls:
- Click + drag: rotate view
- Click face: highlight LEDs

## Implementation Notes

- `Matrix3D`: Python port of Processing transformation matrix
- `test_transforms.pde`: Original Processing reference code
- Point data generation matches hardware PCB layout:
  - 12 pentagon PCBs (104 LEDs each)
  - ~13cm diameter dodecahedron
  - Verified against physical measurements

## Model Generation Process

The model generation process:
1. Reads a YAML model definition file that specifies face types, face instances, and model metadata
2. Loads PCB pick-and-place data to get LED positions
3. Transforms LED positions based on face positions and rotations
4. Calculates neighbor relationships between LEDs
5. Generates a C++ header file with the complete model definition

See [Model.md](../docs/PixelTheater/Model.md) for more details on the model system.

