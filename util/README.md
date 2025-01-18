# DodecaRGB Utilities

This directory contains Python utilities for the DodecaRGB project, including tools for:

- Generating and validating LED coordinates
- Visualizing the physical model
- Processing PCB pick-and-place files
- Pre-calculation tasks

There is also some legacy Processing code that was originally used to generate the LED coordinates.

## Dependencies

### Python Environment Setup

First, install tcl-tk and Python 3.12 with Tk support:

```bash
# Install tcl-tk
brew install tcl-tk

# Install Python 3.12 with Tk support
brew install python@3.12

# Verify Python installation
brew info python@3.12  # Should show that it was built with tcl-tk
```

### Virtual Environment Setup

Create a new virtual environment using the Homebrew Python installation:

```bash
# Create virtual environment (make sure to use the Homebrew Python binary)
/opt/homebrew/opt/python@3.12/bin/python3.12 -m venv venv

# Activate the virtual environment
source venv/bin/activate

# Verify you're using the correct Python version
python --version  # Should show Python 3.12.x
which python     # Should point to your venv Python

# Install dependencies
pip install -r requirements.txt
```

### Troubleshooting

If you encounter Tkinter issues:

Check if the Tkinter module is installed: `python -c "import tkinter; tkinter._test()"`

If issues persist, you can try using the system Tk:

```bash
brew unlink python-tk
brew uninstall python-tk@3.12
brew install python@3.12 --with-tcl-tk
```

## Usage

To parse the pick-and-place file and calculate all the LED position data:

```bash
python generate_points.py
```

After this, the definitions for `Points` in `/src/points.cpp`  will be created in the `data` directory. You will need to copy and paste the updated points into the `src/points.cpp` file. In addition, the script will generate a json file in the `data` directory that contains the LED coordinates, for any other use cases.
