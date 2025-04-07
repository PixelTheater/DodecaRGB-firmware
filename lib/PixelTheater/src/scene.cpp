#include "PixelTheater/scene.h"
#include "PixelTheater/core/imodel.h"
#include "PixelTheater/core/iled_buffer.h"
#include "PixelTheater/platform/platform.h"
#include "PixelTheater/core/log.h"

// Implementation moved to header file since it's a template class
namespace PixelTheater {

// --- Dummy Implementations for Error Returns --- 

// class DummyModel : public IModel {
// public:
//     ~DummyModel() override = default;
//     const Point& point(size_t index) const override {
//         static const Point dummy; return dummy;
//     }
//     size_t pointCount() const noexcept override { return 0; }
//     const Face& face(size_t index) const override {
//         static const Face dummy; return dummy;
//     }
//     size_t faceCount() const noexcept override { return 0; }
// };
// static DummyModel dummy_model_instance;

// class DummyScene : public Scene {
// public:
//     DummyScene() { set_name("DummyScene"); }
//     void setup() override {} // Provide empty implementation
//     void tick() override {}
// };
// static DummyScene dummy_scene_instance;


// --- Scene Method Implementations --- 

void Scene::connect(IModel& model_ref, ILedBuffer& leds_ref, Platform& platform_ref) {
    model_ptr = &model_ref;
    leds_ptr = &leds_ref;
    platform_ptr = &platform_ref;
    leds = LedsProxy(leds_ptr); 
}

const IModel& Scene::model() const {
    if (!model_ptr) {
        logError("Scene::model() called before model connected");
        assert(model_ptr != nullptr && "Scene::model() called before model connected");
        // Cannot return dummy - assert handles error case in debug
        // Fallback for release: requires careful thought if reachable
    }
    return *model_ptr;
}

// Other non-inline Scene methods could go here if needed in the future.

}