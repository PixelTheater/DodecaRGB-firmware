#include "identify_sides_scene.h"
#include "calibration/calibration_controller.h"
#include "calibration/calibration_types.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstdio>

namespace Scenes {

namespace {

// PCB-local edge LED indices from DodecaRGBv2_1 LED_GROUPS (9 LEDs each).
constexpr uint16_t kEdgeLeds[Calibration::kEdgeCount][9] = {
    {130, 131, 133, 134, 80, 81, 82, 84, 86},
    {86, 87, 89, 90, 91, 92, 93, 95, 97},
    {97, 98, 100, 101, 102, 103, 104, 106, 108},
    {108, 109, 111, 112, 113, 114, 115, 117, 119},
    {119, 120, 122, 123, 124, 125, 126, 128, 130},
};

const CRGB kFaceColors[12] = {
    CRGB::Red, CRGB::Lime, CRGB::Blue, CRGB::Yellow,
    CRGB::Magenta, CRGB::Cyan, CRGB::Orange, CRGB::Purple,
    CRGB::White, CRGB::Pink, CRGB(0, 128, 0), CRGB(139, 69, 19)
};

void lightLocalLeds(
    PixelTheater::Scene& scene,
    Calibration::WiringIndex wiring,
    const uint16_t* localIndices,
    size_t count,
    CRGB color
) {
    const uint16_t base = Calibration::wiringLedOffset(wiring);
    for (size_t i = 0; i < count; ++i) {
        const uint16_t local = localIndices[i];
        if (local >= Calibration::kLedsPerFace) {
            continue;
        }
        scene.leds[base + local] = color;
    }
}

void lightCenterCount(
    PixelTheater::Scene& scene,
    Calibration::WiringIndex wiring,
    uint8_t n,
    CRGB color
) {
    const uint16_t base = Calibration::wiringLedOffset(wiring);
    const uint8_t count = static_cast<uint8_t>(std::min<size_t>(n, Calibration::kLedsPerFace));
    for (uint8_t i = 0; i < count; ++i) {
        scene.leds[base + i] = color;
    }
}

void lightEdgeCount(
    PixelTheater::Scene& scene,
    Calibration::WiringIndex wiring,
    uint8_t logicalEdge,
    Calibration::RotationStep rotation,
    uint8_t n,
    CRGB color
) {
    const uint8_t edge = static_cast<uint8_t>(
        (logicalEdge + rotation) % Calibration::kEdgeCount
    );
    const uint8_t count = static_cast<uint8_t>(std::min<size_t>(n, 9));
    lightLocalLeds(scene, wiring, kEdgeLeds[edge], count, color);
}

} // namespace

void IdentifySidesScene::setup() {
    set_name("Identify Sides");
    set_author("DodecaRGB");
    set_description("Face/edge identity map, or interactive calibration walk.");
    set_version("1.1");

    param("Speed", "ratio", 0.0f, 2.0f, DEFAULT_SPEED, "clamp", "Pulse speed (full-map mode)");
    param("Brightness", "ratio", 0.1f, 1.0f, DEFAULT_BRIGHTNESS, "clamp", "Overall brightness");

    logInfo("IdentifySidesScene ready (interactive via calibration.begin)");
}

void IdentifySidesScene::tick() {
    const float speed = settings["Speed"];
    const float brightness = settings["Brightness"];

    for (size_t i = 0; i < ledCount(); ++i) {
        leds[i] = CRGB::Black;
    }

    if (Calibration::Controller::instance().interactive()) {
        tickInteractiveWalk(brightness);
    } else {
        tickFullMap(speed, brightness);
    }
}

void IdentifySidesScene::tickInteractiveWalk(float brightness) {
    auto& cal = Calibration::Controller::instance();
    const uint8_t brightness_factor = static_cast<uint8_t>(brightness * 255.0f);

    const Calibration::WiringIndex focus = cal.focusWiring();
    const Calibration::FaceAssignment& focusA = cal.assignment(focus);

    // Soft fill for all faces so the sculpture stays readable
    for (Calibration::WiringIndex w = 0; w < Calibration::kFaceCount; ++w) {
        const Calibration::FaceAssignment& a = cal.assignment(w);
        CRGB c = kFaceColors[(a.boardLabel - 1) % 12];
        c.nscale8(static_cast<uint8_t>(brightness_factor * 0.12f));
        lightCenterCount(*this, w, 1, c);
    }

    // Anchor (wiring 0 / Side 1): always show label count
    {
        const auto& anchor = cal.assignment(0);
        CRGB c = kFaceColors[(anchor.boardLabel - 1) % 12];
        c.nscale8(brightness_factor);
        lightCenterCount(*this, 0, anchor.boardLabel, c);

        // Edge on anchor facing the focus (logical edge 0 + anchor rotation)
        CRGB ec = kFaceColors[(focusA.boardLabel - 1) % 12];
        ec.nscale8(brightness_factor);
        lightEdgeCount(*this, 0, 0, anchor.rotationStep, focusA.boardLabel, ec);
    }

    // Focus face: center = its boardLabel count; edge toward anchor
    {
        CRGB c = kFaceColors[(focusA.boardLabel - 1) % 12];
        c.nscale8(brightness_factor);
        lightCenterCount(*this, focus, focusA.boardLabel, c);

        const auto& anchor = cal.assignment(0);
        CRGB ec = kFaceColors[(anchor.boardLabel - 1) % 12];
        ec.nscale8(brightness_factor);
        lightEdgeCount(*this, focus, 0, focusA.rotationStep, anchor.boardLabel, ec);
    }

    // Pulse focus slightly
    const float pulse = 0.55f + 0.45f * (0.5f + 0.5f * sinf(millis() / 200.0f));
    const uint16_t base = Calibration::wiringLedOffset(focus);
    for (uint16_t i = 0; i < Calibration::kLedsPerFace; ++i) {
        leds[base + i].nscale8(static_cast<uint8_t>(pulse * 255.0f));
    }
}

void IdentifySidesScene::tickFullMap(float speed, float brightness) {
    const uint8_t brightness_factor = static_cast<uint8_t>(brightness * 255.0f);
    const uint8_t total_faces = model().faceCount();

    CRGB face_colors[12];
    for (size_t face_id = 0; face_id < total_faces; face_id++) {
        face_colors[face_id] = kFaceColors[face_id % 12];
    }

    const float time_seconds = millis() / 1000.0f;
    const float color_duration = 3.0f;
    const uint8_t current_pulsing_color =
        static_cast<uint8_t>(fmod(time_seconds / color_duration, total_faces));
    const float pulse_freq = 1.0f * speed;
    const float pulse_phase = time_seconds * pulse_freq * 2.0f * 3.14159f;
    const float pulse_factor = 0.3f + 0.7f * (0.5f + 0.5f * sinf(pulse_phase));

    for (size_t geometric_pos = 0; geometric_pos < total_faces; geometric_pos++) {
        auto face = model().face(geometric_pos);
        CRGB face_color = face_colors[geometric_pos];
        face_color.nscale8(brightness_factor);
        if (geometric_pos == current_pulsing_color) {
            face_color.nscale8(static_cast<uint8_t>(pulse_factor * 255.0f));
        }
        const size_t leds_to_light =
            std::min(static_cast<size_t>(geometric_pos + 1), static_cast<size_t>(face.led_count()));
        for (size_t led_idx = 0; led_idx < leds_to_light; led_idx++) {
            face.leds[led_idx] = face_color;
        }
    }

    for (size_t geometric_pos = 0; geometric_pos < total_faces; geometric_pos++) {
        const auto& face = model().face(geometric_pos);
        const uint8_t num_edges = model().face_edge_count(geometric_pos);

        for (uint8_t edge_idx = 0; edge_idx < num_edges; edge_idx++) {
            const int8_t adjacent_geometric_pos = model().face_at_edge(geometric_pos, edge_idx);
            CRGB edge_color;
            uint8_t edge_face_id = 0;

            if (adjacent_geometric_pos >= 0 &&
                adjacent_geometric_pos < static_cast<int8_t>(total_faces)) {
                edge_color = face_colors[adjacent_geometric_pos];
                edge_face_id = static_cast<uint8_t>(adjacent_geometric_pos);
            } else {
                edge_face_id = static_cast<uint8_t>((geometric_pos + edge_idx + 1) % total_faces);
                edge_color = face_colors[edge_face_id];
            }
            edge_color.nscale8(brightness_factor);
            if (edge_face_id == current_pulsing_color) {
                edge_color.nscale8(static_cast<uint8_t>(pulse_factor * 255.0f));
            }

            if (face.vertices.size() <= edge_idx || face.vertices.size() == 0) {
                continue;
            }

            const auto& v0 = face.vertices[edge_idx];
            const auto& v1 = face.vertices[(edge_idx + 1) % face.vertices.size()];
            const float edge_center_x = (v0.x + v1.x) / 2.0f;
            const float edge_center_y = (v0.y + v1.y) / 2.0f;
            const float edge_center_z = (v0.z + v1.z) / 2.0f;

            struct LedDistance {
                float distance;
                uint16_t led_index;
            };
            std::vector<LedDistance> led_distances;
            led_distances.reserve(face.led_count());

            for (uint16_t face_led_idx = 0; face_led_idx < face.led_count(); face_led_idx++) {
                const uint16_t global_led_idx = face.led_offset() + face_led_idx;
                const auto& led_point = model().point(global_led_idx);
                const float dx = led_point.x() - edge_center_x;
                const float dy = led_point.y() - edge_center_y;
                const float dz = led_point.z() - edge_center_z;
                led_distances.push_back({std::sqrt(dx * dx + dy * dy + dz * dz), global_led_idx});
            }

            std::sort(
                led_distances.begin(),
                led_distances.end(),
                [](const LedDistance& a, const LedDistance& b) { return a.distance < b.distance; }
            );

            const uint8_t num_leds_to_light = (adjacent_geometric_pos >= 0)
                ? static_cast<uint8_t>(adjacent_geometric_pos + 1)
                : static_cast<uint8_t>(edge_face_id + 1);
            const size_t leds_to_use =
                std::min(static_cast<size_t>(num_leds_to_light), led_distances.size());

            for (size_t i = 0; i < leds_to_use; i++) {
                leds[led_distances[i].led_index] = edge_color;
            }
        }
    }
}

std::string IdentifySidesScene::status() const {
    auto& cal = Calibration::Controller::instance();
    if (cal.interactive()) {
        char buffer[160];
        const auto& a = cal.assignment(cal.focusWiring());
        snprintf(
            buffer,
            sizeof(buffer),
            "WALK w%u label=%u slot=%u rot=%u  [A/D rot, W/S shift, Enter confirm]",
            static_cast<unsigned>(cal.focusWiring()),
            static_cast<unsigned>(a.boardLabel),
            static_cast<unsigned>(a.slotIndex),
            static_cast<unsigned>(a.rotationStep)
        );
        return std::string(buffer);
    }

    const float speed = settings["Speed"];
    const float brightness = settings["Brightness"];
    const float time_seconds = millis() / 1000.0f;
    const uint8_t current_pulsing_face =
        static_cast<uint8_t>(fmod(time_seconds / 3.0f, model().faceCount()));
    char buffer[128];
    snprintf(
        buffer,
        sizeof(buffer),
        "MAP pulse=%u Spd:%.1f Bri:%.1f (send calibration.begin)",
        current_pulsing_face,
        speed,
        brightness
    );
    return std::string(buffer);
}

} // namespace Scenes
