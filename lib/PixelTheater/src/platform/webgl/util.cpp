#include "PixelTheater/platform/webgl/util.h"
#include <emscripten.h>

namespace PixelTheater {
namespace WebGLUtil {

// External JavaScript functions (declared in JavaScript code)
extern "C" {
    // Canvas dimensions
    extern int get_canvas_width();
    extern int get_canvas_height();
    
    // UI interaction
    extern void update_fps_counter(int fps);
}

int getCanvasWidth() {
    return get_canvas_width();
}

int getCanvasHeight() {
    return get_canvas_height();
}

void updateFPSCounter(int fps) {
    update_fps_counter(fps);
}

} // namespace WebGLUtil
} // namespace PixelTheater 