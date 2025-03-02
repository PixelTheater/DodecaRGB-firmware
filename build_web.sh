#!/bin/bash
# build_web.sh

# Create web directory if it doesn't exist
mkdir -p web

# Check if the version file exists and is causing problems
if [ -f "./version" ]; then
  echo "Found problematic version file. Moving it temporarily..."
  mv ./version ./version.bak
fi

echo "Building WebGL LED Simulator with debugging options..."

# Compile the C++ code to WebAssembly with debugging information
emcc src/web_simulator.cpp \
     src/benchmark.cpp \
     src/math_provider.cpp \
     lib/PixelTheater/src/core/color.cpp \
     lib/PixelTheater/src/core/crgb.cpp \
     lib/PixelTheater/src/model/point.cpp \
     lib/PixelTheater/src/palette.cpp \
     lib/PixelTheater/src/params/handlers/flag_handler.cpp \
     lib/PixelTheater/src/params/handlers/type_handler.cpp \
     lib/PixelTheater/src/params/param_types.cpp \
     lib/PixelTheater/src/platform/web_platform.cpp \
     lib/PixelTheater/src/settings.cpp \
     lib/PixelTheater/src/stage.cpp \
     src/model.cpp \
     -I"." \
     -I"lib" \
     -I"lib/PixelTheater/include" \
     -I"src" \
     -I"src/models" \
     -I"src/scenes" \
     -I".pio/libdeps/web/ArduinoEigen/ArduinoEigen" \
     -I"include" \
     -std=c++17 \
     -DPLATFORM_WEB \
     -DEMSCRIPTEN \
     -DDEBUG \
     -s WASM=1 \
     -s USE_WEBGL2=1 \
     -s FULL_ES3=1 \
     -s ALLOW_MEMORY_GROWTH=1 \
     -s EXPORTED_RUNTIME_METHODS='["UTF8ToString", "ccall", "cwrap"]' \
     -s EXPORTED_FUNCTIONS='["_main", "_change_scene", "_get_scene_count", "_get_scene_name", "_set_brightness", "_show_benchmark_report", "_toggle_debug_mode", "_print_model_info"]' \
     -s INITIAL_MEMORY=32MB \
     -s MAXIMUM_MEMORY=128MB \
     -s ASSERTIONS=2 \
     -s SAFE_HEAP=1 \
     -s GL_DEBUG=1 \
     -s DEMANGLE_SUPPORT=1 \
     -s STACK_OVERFLOW_CHECK=2 \
     -g4 \
     -O0 \
     --source-map-base ./ \
     -o web/simulator.js

# Check if the build was successful
if [ $? -eq 0 ]; then
  echo "Build successful!"
else
  echo "Build failed!"
  exit 1
fi

# Restore the version file if we moved it
if [ -f "./version.bak" ]; then
  echo "Restoring version file..."
  mv ./version.bak ./version
fi

echo "Build complete. Files are in the web/ directory."
echo "Run a local server to test: cd web && python -m http.server"
