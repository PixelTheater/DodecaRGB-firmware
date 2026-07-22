#pragma once

#include <Arduino.h>
#include "calibration_controller.h"

namespace Calibration {

/**
 * Minimal NDJSON request/response on Serial.
 * One JSON object per line. Unknown methods return an error object.
 */
class SerialProtocol {
public:
    void begin(Stream& stream);
    void poll(); // call every loop; may mute when quiet

private:
    Stream* _stream = nullptr;
    char _line[384];
    size_t _len = 0;

    void handleLine(const char* line);
    void replyResult(int id, const char* jsonObjectBody);
    void replyError(int id, const char* code, const char* message);
    void replyFocus(int id, Controller& cal, bool ok);
    void streamCalibrationGet(int id, Controller& cal);
    static int extractId(const char* line);
    static bool extractMethod(const char* line, char* out, size_t outSize);
    static int extractIntParam(const char* line, const char* key, int defaultValue);
};

} // namespace Calibration
