# Makefile for WebGL LED Simulator

# Directories
SRC_DIR = src
LIB_DIR = lib
WEB_DIR = web
PIXELTHEATER_DIR = $(LIB_DIR)/PixelTheater

# Include paths
INCLUDES = -I"." \
           -I"$(LIB_DIR)" \
           -I"$(PIXELTHEATER_DIR)/include" \
           -I"$(SRC_DIR)" \
           -I"$(SRC_DIR)/models" \
           -I"$(SRC_DIR)/scenes" \
           -I".pio/libdeps/web/ArduinoEigen/ArduinoEigen" \
					 -I"include"

# Source files
SOURCES = $(SRC_DIR)/web_simulator.cpp \
					$(SRC_DIR)/benchmark.cpp \
					$(SRC_DIR)/math_provider.cpp \
          $(PIXELTHEATER_DIR)/src/core/color.cpp \
          $(PIXELTHEATER_DIR)/src/core/crgb.cpp \
          $(PIXELTHEATER_DIR)/src/model/point.cpp \
          $(PIXELTHEATER_DIR)/src/palette.cpp \
          $(PIXELTHEATER_DIR)/src/params/handlers/flag_handler.cpp \
          $(PIXELTHEATER_DIR)/src/params/handlers/type_handler.cpp \
          $(PIXELTHEATER_DIR)/src/params/param_types.cpp \
          $(PIXELTHEATER_DIR)/src/platform/web_platform.cpp \
          $(PIXELTHEATER_DIR)/src/settings.cpp \
          $(PIXELTHEATER_DIR)/src/stage.cpp \
          $(SRC_DIR)/model.cpp \
          $(PIXELTHEATER_DIR)/src/platform/webgl/util.cpp \
          $(PIXELTHEATER_DIR)/src/platform/webgl/camera.cpp \
          $(PIXELTHEATER_DIR)/src/platform/webgl/mesh.cpp \
          $(PIXELTHEATER_DIR)/src/platform/webgl/shaders.cpp \
          $(PIXELTHEATER_DIR)/src/platform/webgl/renderer.cpp

# Emscripten flags
EMCC_FLAGS = -std=c++17 \
             -DPLATFORM_WEB \
             -DEMSCRIPTEN \
             -s WASM=1 \
             -s USE_WEBGL2=1 \
             -s FULL_ES3=1 \
             -s ALLOW_MEMORY_GROWTH=1 \
             -s EXPORTED_RUNTIME_METHODS='["UTF8ToString"]' \
             -s EXPORTED_FUNCTIONS='["_main", "_change_scene", "_get_scene_count", "_get_scene_name", "_set_brightness"]'

# Output files
OUTPUT = $(WEB_DIR)/simulator.js

# Default target
all: $(WEB_DIR) $(OUTPUT)

# Create web directory if it doesn't exist
$(WEB_DIR):
	mkdir -p $(WEB_DIR)

# Build the WebAssembly module
$(OUTPUT): $(SOURCES)
	emcc $(SOURCES) $(INCLUDES) $(EMCC_FLAGS) -o $(OUTPUT)

# Simple test build
simple: $(WEB_DIR)
	emcc $(SRC_DIR)/web_simulator_simple.cpp -std=c++17 -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_RUNTIME_METHODS='["UTF8ToString"]' -s EXPORTED_FUNCTIONS='["_main", "_set_brightness"]' -o $(WEB_DIR)/simulator_simple.js

# Clean build artifacts
clean:
	rm -f $(OUTPUT) $(WEB_DIR)/simulator.wasm

# Run a local server
serve:
	cd $(WEB_DIR) && python -m http.server

.PHONY: all clean serve simple 