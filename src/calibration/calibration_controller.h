#pragma once

#include "calibration_types.h"
#include <Arduino.h>

namespace Calibration {

/**
 * In-memory assembly map + interactive walk state.
 * W/S swap the focused wiring entry with next/prev in the addressable chain.
 * A/D change rotationStep on the focused entry.
 * Side 1 (wiring 0) is the anchor and cannot be shifted.
 */
class Controller {
public:
    static Controller& instance();

    void resetToFactory();
    void beginInteractive();
    void endInteractive();
    bool interactive() const { return _interactive; }
    bool quietSerial() const { return _quietSerial; }
    void setQuietSerial(bool quiet) { _quietSerial = quiet; }

    const FaceAssignment* assignments() const { return _assignments; }
    FaceAssignment& assignment(WiringIndex w) { return _assignments[w]; }
    const FaceAssignment& assignment(WiringIndex w) const { return _assignments[w]; }

    WiringIndex focusWiring() const { return _focusWiring; }
    void setFocusWiring(WiringIndex w);

    // dir +1 = W (swap with next), -1 = S (swap with previous). Focus follows the moved entry.
    bool shiftAlongWiring(int dir);

    // dir +1 = D (CW), -1 = A (CCW) as viewed from outside the face.
    bool rotateFocus(int dir);

    bool confirmFocus(); // mark resolved, advance to next unresolved after anchor
    void clearResolved();
    bool isResolved(WiringIndex w) const { return _resolved[w]; }
    size_t resolvedCount() const;

    // Find wiring index that currently carries a board label (1..12), or -1.
    int wiringForBoardLabel(BoardLabel label) const;

    void printStatus(Stream& out) const;

private:
    Controller();
    FaceAssignment _assignments[kFaceCount];
    bool _resolved[kFaceCount]{};
    WiringIndex _focusWiring = 1; // start after anchor
    bool _interactive = false;
    bool _quietSerial = false;
};

} // namespace Calibration
