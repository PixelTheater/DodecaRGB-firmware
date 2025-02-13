#pragma once

namespace PixelTheater {
namespace ParamHandlers {

class SentinelHandler {
public:
    template<typename T>
    static T get_sentinel() {
        if constexpr (std::is_same_v<T, float>) return 0.0f;
        if constexpr (std::is_same_v<T, int>) return -1;
        if constexpr (std::is_same_v<T, bool>) return false;
        return T{};  // Default construct other types
    }

    template<typename T>
    static bool is_sentinel(const T& value) {
        if constexpr (std::is_same_v<T, float>) return value == 0.0f;
        if constexpr (std::is_same_v<T, int>) return value == -1;
        if constexpr (std::is_same_v<T, bool>) return value == false;
        return false;
    }
};

}
} // namespace PixelTheater::ParamHandlers 