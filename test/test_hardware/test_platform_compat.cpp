#include <doctest/doctest.h>
#include <Arduino.h>
#include <array>
#include <vector>
#include <span>

TEST_CASE("Platform Compatibility") {
    Serial.println("\n=== Testing Platform Compatibility ===");

    SUBCASE("std::array") {
        Serial.println("Testing std::array...");
        std::array<int, 5> arr = {1, 2, 3, 4, 5};
        CHECK(arr.size() == 5);
        CHECK(arr[0] == 1);
        
        // Test array iteration
        int sum = 0;
        for(const auto& val : arr) {
            sum += val;
        }
        CHECK(sum == 15);
    }

    SUBCASE("array_view compatibility") {
        Serial.println("Testing array_view alternative...");
        int raw_array[] = {1, 2, 3, 4, 5};
        
        // Test our array_view implementation
        struct array_view {
            const int* _data;
            size_t _size;
            array_view(const int* data, size_t size) : _data(data), _size(size) {}
            size_t size() const { return _size; }
            const int& operator[](size_t i) const { return i < _size ? _data[i] : _data[0]; }
            const int* begin() const { return _data; }
            const int* end() const { return _data + _size; }
        };

        array_view view(raw_array, 5);
        CHECK(view.size() == 5);
        CHECK(view[0] == 1);
        
        // Test view iteration
        int sum = 0;
        for(const auto& val : view) {
            sum += val;
        }
        CHECK(sum == 15);
    }

    SUBCASE("constexpr evaluation") {
        Serial.println("Testing constexpr...");
        constexpr int arr_size = 5;
        std::array<int, arr_size> arr = {1, 2, 3, 4, 5};
        CHECK(arr.size() == arr_size);
    }

    SUBCASE("template features") {
        Serial.println("Testing templates...");
        // Test if template deduction guides work
        std::array arr{1, 2, 3, 4, 5};  // C++17 deduction
        CHECK(arr.size() == 5);
    }

    SUBCASE("memory alignment") {
        Serial.println("Testing memory alignment...");
        // Test if alignas works
        alignas(16) int aligned_var = 42;
        CHECK(reinterpret_cast<uintptr_t>(&aligned_var) % 16 == 0);
    }

    SUBCASE("STL containers") {
        Serial.println("Testing STL containers...");
        
        // Test vector availability and behavior
        std::vector<int> vec = {1, 2, 3};
        CHECK(vec.size() == 3);
        vec.push_back(4);
        CHECK(vec.size() == 4);
        
        // Test initializer lists
        std::initializer_list<int> init = {1, 2, 3};
        CHECK(std::distance(init.begin(), init.end()) == 3);
    }

    SUBCASE("move semantics") {
        Serial.println("Testing move semantics...");
        std::array<int, 3> arr1 = {1, 2, 3};
        auto arr2 = std::move(arr1);  // Should compile and work
        CHECK(arr2[0] == 1);
    }

    SUBCASE("reference types") {
        Serial.println("Testing reference types...");
        int x = 42;
        int& ref = x;
        CHECK(&ref == &x);  // Reference works as expected
        
        // Test reference in range-based for
        std::array<int, 3> arr = {1, 2, 3};
        for(int& val : arr) {
            val *= 2;  // Modify through reference
        }
        CHECK(arr[0] == 2);
    }

    SUBCASE("C++20 features") {
        Serial.println("Testing C++20 features...");
        Serial.printf("C++ version: %ld\n", __cplusplus);
        
        #ifdef __cpp_lib_span
        Serial.println("std::span is supported");
        int raw_array[] = {1, 2, 3, 4, 5};
        std::span<int> span(raw_array);
        CHECK(span.size() == 5);
        #else
        Serial.println("std::span is NOT supported");
        Serial.printf("Required C++ feature test macro __cpp_lib_span not defined\n");
        #endif
    }

    Serial.println("Platform compatibility tests complete!");
} 