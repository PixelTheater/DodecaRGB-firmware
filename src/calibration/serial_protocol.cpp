#include "serial_protocol.h"
#include <cstring>
#include <cstdlib>

namespace Calibration {

void SerialProtocol::begin(Stream& stream) {
    _stream = &stream;
    _len = 0;
}

void SerialProtocol::poll() {
    if (!_stream) {
        return;
    }
    while (_stream->available() > 0) {
        const char c = static_cast<char>(_stream->read());
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            _line[_len] = '\0';
            if (_len > 0) {
                handleLine(_line);
            }
            _len = 0;
            continue;
        }
        if (_len + 1 < sizeof(_line)) {
            _line[_len++] = c;
        } else {
            _len = 0; // overflow: drop line
        }
    }
}

int SerialProtocol::extractId(const char* line) {
    const char* p = strstr(line, "\"id\"");
    if (!p) {
        return 0;
    }
    p = strchr(p, ':');
    if (!p) {
        return 0;
    }
    return atoi(p + 1);
}

bool SerialProtocol::extractMethod(const char* line, char* out, size_t outSize) {
    const char* p = strstr(line, "\"method\"");
    if (!p) {
        return false;
    }
    p = strchr(p, ':');
    if (!p) {
        return false;
    }
    p = strchr(p, '"');
    if (!p) {
        return false;
    }
    ++p;
    const char* end = strchr(p, '"');
    if (!end || end <= p) {
        return false;
    }
    size_t n = static_cast<size_t>(end - p);
    if (n >= outSize) {
        n = outSize - 1;
    }
    memcpy(out, p, n);
    out[n] = '\0';
    return true;
}

int SerialProtocol::extractIntParam(const char* line, const char* key, int defaultValue) {
    char pattern[48];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char* p = strstr(line, pattern);
    if (!p) {
        return defaultValue;
    }
    p = strchr(p, ':');
    if (!p) {
        return defaultValue;
    }
    return atoi(p + 1);
}

void SerialProtocol::replyResult(int id, const char* jsonObjectBody) {
    if (!_stream) {
        return;
    }
    _stream->printf("{\"id\":%d,\"result\":%s}\n", id, jsonObjectBody);
}

void SerialProtocol::replyError(int id, const char* code, const char* message) {
    if (!_stream) {
        return;
    }
    _stream->printf(
        "{\"id\":%d,\"error\":{\"code\":\"%s\",\"message\":\"%s\"}}\n",
        id, code, message
    );
}

void SerialProtocol::replyFocus(int id, Controller& cal, bool ok) {
    const FaceAssignment& a = cal.assignment(cal.focusWiring());
    char body[160];
    snprintf(
        body,
        sizeof(body),
        "{\"ok\":%s,\"focusWiring\":%u,\"boardLabel\":%u,\"slotIndex\":%u,"
        "\"rotationStep\":%u,\"resolved\":%u}",
        ok ? "true" : "false",
        static_cast<unsigned>(cal.focusWiring()),
        static_cast<unsigned>(a.boardLabel),
        static_cast<unsigned>(a.slotIndex),
        static_cast<unsigned>(a.rotationStep),
        static_cast<unsigned>(cal.resolvedCount())
    );
    replyResult(id, body);
}

void SerialProtocol::streamCalibrationGet(int id, Controller& cal) {
    if (!_stream) {
        return;
    }
    // Stream full payload — a fixed 512-byte body was truncating around face 8/9.
    _stream->printf(
        "{\"id\":%d,\"result\":{\"focusWiring\":%u,\"resolved\":%u,\"assignments\":[",
        id,
        static_cast<unsigned>(cal.focusWiring()),
        static_cast<unsigned>(cal.resolvedCount())
    );
    for (size_t w = 0; w < kFaceCount; ++w) {
        const FaceAssignment& a = cal.assignment(static_cast<WiringIndex>(w));
        _stream->printf(
            "%s{\"w\":%u,\"boardLabel\":%u,\"slotIndex\":%u,\"rotationStep\":%u}",
            (w == 0 ? "" : ","),
            static_cast<unsigned>(w),
            static_cast<unsigned>(a.boardLabel),
            static_cast<unsigned>(a.slotIndex),
            static_cast<unsigned>(a.rotationStep)
        );
    }
    _stream->print("]}}\n");
}

void SerialProtocol::handleLine(const char* line) {
    if (line[0] != '{') {
        return;
    }

    const int id = extractId(line);
    char method[48];
    if (!extractMethod(line, method, sizeof(method))) {
        replyError(id, "BAD_REQUEST", "missing method");
        return;
    }

    Controller& cal = Controller::instance();

    if (strcmp(method, "device.getInfo") == 0) {
        replyResult(id,
            "{\"firmwareVersion\":\"0.2.2\",\"protocolVersion\":1,"
            "\"modelId\":\"DodecaRGBv2_1\",\"calibrationInteractive\":true}");
        return;
    }

    if (strcmp(method, "calibration.begin") == 0) {
        cal.beginInteractive();
        replyFocus(id, cal, true);
        return;
    }

    if (strcmp(method, "calibration.end") == 0) {
        cal.endInteractive();
        replyResult(id, "{\"ok\":true}");
        return;
    }

    if (strcmp(method, "calibration.get") == 0) {
        streamCalibrationGet(id, cal);
        return;
    }

    if (strcmp(method, "calibration.rotate") == 0) {
        const int dir = extractIntParam(line, "dir", 1);
        if (!cal.interactive()) {
            replyError(id, "NOT_ACTIVE", "call calibration.begin first");
            return;
        }
        replyFocus(id, cal, cal.rotateFocus(dir));
        return;
    }

    if (strcmp(method, "calibration.shift") == 0) {
        const int dir = extractIntParam(line, "dir", 1);
        if (!cal.interactive()) {
            replyError(id, "NOT_ACTIVE", "call calibration.begin first");
            return;
        }
        replyFocus(id, cal, cal.shiftAlongWiring(dir));
        return;
    }

    if (strcmp(method, "calibration.setFocus") == 0) {
        const int w = extractIntParam(line, "wiringIndex", -1);
        if (w < 0 || w >= static_cast<int>(kFaceCount)) {
            replyError(id, "BAD_PARAM", "wiringIndex out of range");
            return;
        }
        cal.setFocusWiring(static_cast<WiringIndex>(w));
        replyFocus(id, cal, true);
        return;
    }

    if (strcmp(method, "calibration.confirm") == 0) {
        if (!cal.interactive()) {
            replyError(id, "NOT_ACTIVE", "call calibration.begin first");
            return;
        }
        cal.confirmFocus();
        replyFocus(id, cal, true);
        return;
    }

    if (strcmp(method, "calibration.reset") == 0) {
        cal.resetToFactory();
        if (cal.interactive()) {
            cal.beginInteractive();
        }
        replyFocus(id, cal, true);
        return;
    }

    replyError(id, "UNKNOWN_METHOD", method);
}

} // namespace Calibration
