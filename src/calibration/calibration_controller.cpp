#include "calibration_controller.h"

namespace Calibration {

Controller& Controller::instance() {
    static Controller c;
    return c;
}

Controller::Controller() {
    resetToFactory();
}

void Controller::resetToFactory() {
    initFactoryAssignments(_assignments);
    clearResolved();
    _resolved[0] = true; // Side 1 / wiring 0 is the locked anchor
    _focusWiring = 1;
}

void Controller::beginInteractive() {
    _interactive = true;
    _quietSerial = true;
    if (_focusWiring == 0) {
        _focusWiring = 1;
    }
}

void Controller::endInteractive() {
    _interactive = false;
    _quietSerial = false;
}

void Controller::setFocusWiring(WiringIndex w) {
    if (w >= kFaceCount) {
        return;
    }
    // Do not focus the locked anchor for shift operations; allow view-only at 0 via explicit set
    _focusWiring = w;
}

bool Controller::shiftAlongWiring(int dir) {
    if (!_interactive || dir == 0) {
        return false;
    }
    // Anchor (wiring 0 / Side 1) cannot move
    if (_focusWiring == 0) {
        return false;
    }

    int target = static_cast<int>(_focusWiring) + (dir > 0 ? 1 : -1);
    if (target <= 0 || target >= static_cast<int>(kFaceCount)) {
        return false; // cannot swap with or past anchor / end
    }

    const WiringIndex a = _focusWiring;
    const WiringIndex b = static_cast<WiringIndex>(target);
    FaceAssignment tmp = _assignments[a];
    _assignments[a] = _assignments[b];
    _assignments[b] = tmp;

    // Resolved flags stay with wiring slots (physical chain positions), not with labels
    // Focus follows the moved content (bubble along the list)
    _focusWiring = b;
    return true;
}

bool Controller::rotateFocus(int dir) {
    if (!_interactive || dir == 0) {
        return false;
    }
    FaceAssignment& a = _assignments[_focusWiring];
    int step = static_cast<int>(a.rotationStep) + (dir > 0 ? 1 : -1);
    while (step < 0) {
        step += static_cast<int>(kOrientationCount);
    }
    a.rotationStep = static_cast<RotationStep>(step % static_cast<int>(kOrientationCount));
    return true;
}

bool Controller::confirmFocus() {
    if (!_interactive) {
        return false;
    }
    _resolved[_focusWiring] = true;

    // Advance to next unresolved after current, wrapping but skipping anchor if already resolved
    for (size_t i = 1; i < kFaceCount; ++i) {
        WiringIndex w = static_cast<WiringIndex>((_focusWiring + i) % kFaceCount);
        if (w == 0) {
            continue;
        }
        if (!_resolved[w]) {
            _focusWiring = w;
            return true;
        }
    }
    return true; // all done; focus stays
}

void Controller::clearResolved() {
    for (size_t i = 0; i < kFaceCount; ++i) {
        _resolved[i] = false;
    }
    _resolved[0] = true;
}

size_t Controller::resolvedCount() const {
    size_t n = 0;
    for (size_t i = 0; i < kFaceCount; ++i) {
        if (_resolved[i]) {
            ++n;
        }
    }
    return n;
}

int Controller::wiringForBoardLabel(BoardLabel label) const {
    for (size_t w = 0; w < kFaceCount; ++w) {
        if (_assignments[w].boardLabel == label) {
            return static_cast<int>(w);
        }
    }
    return -1;
}

void Controller::printStatus(Stream& out) const {
    out.printf(
        "cal focus=w%u label=%u slot=%u rot=%u resolved=%u/%u\n",
        static_cast<unsigned>(_focusWiring),
        static_cast<unsigned>(_assignments[_focusWiring].boardLabel),
        static_cast<unsigned>(_assignments[_focusWiring].slotIndex),
        static_cast<unsigned>(_assignments[_focusWiring].rotationStep),
        static_cast<unsigned>(resolvedCount()),
        static_cast<unsigned>(kFaceCount)
    );
}

} // namespace Calibration
