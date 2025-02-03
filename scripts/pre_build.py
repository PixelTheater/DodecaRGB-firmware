Import("env")

import os
import sys
from datetime import datetime
pioenv = env.get("PIOENV", "")
print(f"environment: {pioenv}")

# Get project paths from PlatformIO environment
project_dir = env.get("PROJECT_DIR", ".")
parent_dir = project_dir  # Project root contains util/
sys.path.append(parent_dir)
print(f"Added {parent_dir} to Python path")

def before_build(source, target, env):
    """Pre-build hook to update docs and version info"""
    print("Running pre-build tasks...")
    
    # Read version
    with open(os.path.join(project_dir, "VERSION"), "r") as f:
        version = f.read().strip()
    
    # Set version for build
    env.Append(CPPDEFINES=[
        ("PROJECT_VERSION", f'\\"{version}\\"')
    ])
    # check dependencies
    try:
        import frontmatter
    except ImportError:
        env.Execute("$PYTHONEXE -m pip install python-frontmatter")

    # Run doc builder if it exists
    try:
        from util.doc_builder import DocBuilder
        print("Updating documentation...")
        builder = DocBuilder(os.path.join(project_dir, "docs"))
        builder.process_docs()
    except ImportError as e:
        print(f"Doc builder not available - skipping documentation update: {e}")

if (pioenv == "teensy41"):
  before_build("buildprog","build", env)