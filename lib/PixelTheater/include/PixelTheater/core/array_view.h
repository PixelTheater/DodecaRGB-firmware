#pragma once
#include <cstddef>

namespace PixelTheater {

template<typename T>
class array_view {
    T* _data;
    size_t _size;
public:
    array_view(T* data, size_t size) : _data(data), _size(size) {}
    size_t size() const { return _size; }
    T& operator[](size_t i) { return i < _size ? _data[i] : _data[0]; }
    const T& operator[](size_t i) const { return i < _size ? _data[i] : _data[0]; }
    T* begin() { return _data; }
    T* end() { return _data + _size; }
    const T* begin() const { return _data; }
    const T* end() const { return _data + _size; }
};

} // namespace PixelTheater 