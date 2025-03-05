# Python Utilities

Tools for model generation and development.

## Setup

```bash
pip install -r requirements.txt
```

## Tools

### Model Generation
```bash
# Generate LED coordinates
python generate_points.py [-f json] [-o output.json]

# Generate scene code
python generate_scene.py -h

# Generate model
python generate_model.py -h
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

