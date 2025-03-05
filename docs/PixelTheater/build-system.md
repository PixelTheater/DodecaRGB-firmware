---
category: Development
generated: 2025-02-10 00:32
version: 2.8.2
---

# Build System

## Prerequisites

- PlatformIO Core (CLI) or IDE
- Python 3.8+
- Emscripten SDK (web builds only)
- Git

## Environment Setup

1. Install dependencies:
   ```bash
   pip install -r requirements.txt
   ```

2. Configure environment:
   ```bash
   cp .env.example .env
   cp .envrc.example .envrc  # if using direnv
   ```

## Build Environments

### teensy41
```ini
platform = teensy
board = teensy41
framework = arduino
board_build.f_cpu = 600000000L
```

### native
```ini
platform = native
test_framework = doctest
```

### web
```ini
platform = native
extra_scripts = post:scripts/web_build.py
```

## Build Commands

Build firmware:
```bash
pio run -e teensy41
```

Run tests:
```bash
pio test -e native          # Development machine
pio test -e teensy41        # Hardware tests (requires Teensy)
```

Build web simulator:
```bash
./build_web.sh
```

## Test Configuration

Hardware tests run at 115200 baud and report via Serial. Test environments are isolated:

```
test/
├── test_hardware/     # Teensy-specific tests
│   ├── test_fastled.cpp
│   ├── test_model.cpp
│   └── test_platform_compat.cpp
└── test_native/      # Platform-independent tests
```

## Build Scripts

Pre-build:
```
scripts/
├── pre_build.py          # Environment setup
├── generate_scenes.py    # YAML to C++ scene definitions
└── generate_points.py    # LED coordinate generation
```

Post-build:
```
scripts/
└── web_build.py         # Emscripten/WASM compilation
```

## Project Structure

```
.
├── src/          # Main source
├── lib/          # Libraries
├── include/      # Headers
├── test/         # Test files
├── web/          # Simulator
├── scripts/      # Build scripts
└── util/         # Python utilities
```

## Python Utilities

Run tests:
```bash
python -m util.tests.run_tests
```

Generate scene code:
```bash
python util/generate_scene.py -h
```

Generate LED coordinates:
```bash
python util/generate_points.py [-f json] [-o output.json]
```

Run 3D visualizer:
```bash
python util/visualizer.py
```

The visualizer provides interactive 3D viewing of PCB and LED arrangements. Mouse controls:
- Click + drag: rotate view
- Click face: highlight LEDs
