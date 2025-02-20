#include <doctest/doctest.h>
#include "PixelTheater/core/array_view.h"

using namespace PixelTheater;

TEST_SUITE("array_view") {
    TEST_CASE("const and non-const access") {
        int data[] = {1, 2, 3, 4, 5};
        
        // Test both const and non-const views
        array_view<int> mutable_view(data, 5);
        array_view<const int> const_view(data, 5);
        
        // Verify mutable access
        mutable_view[0] = 10;
        CHECK(data[0] == 10);
        
        // Verify const access compiles
        const auto val = const_view[0];
        CHECK(val == 10);
        
        // Test assignment through reference
        int& ref = mutable_view[0];
        ref = 42;
        CHECK(data[0] == 42);
    }
    
    TEST_CASE("range-based for loops") {
        int data[] = {1, 2, 3, 4, 5};
        array_view<int> view(data, 5);
        
        // Test modification through range-based for
        for(auto& x : view) {
            x *= 2;
        }
        CHECK(data[0] == 2);
        CHECK(data[4] == 10);
        
        // Test const iteration
        int sum = 0;
        for(const auto& x : view) {
            sum += x;
        }
        CHECK(sum == (2 + 4 + 6 + 8 + 10));
    }
    
    TEST_CASE("iterator compatibility") {
        int data[] = {1, 2, 3, 4, 5};
        array_view<const int> view(data, 5);
        
        int sum = 0;
        for(auto it = view.begin(); it != view.end(); ++it) {
            sum += *it;
        }
        CHECK(sum == 15);
    }
    
    TEST_CASE("bounds checking") {
        int data[] = {1, 2, 3, 4, 5};
        array_view<int> view(data, 5);
        
        // Out of bounds access returns first element
        CHECK(view[10] == view[0]);
        CHECK(view[1000] == view[0]);
    }
    
    TEST_CASE("empty views") {
        array_view<int> empty(nullptr, 0);
        CHECK(empty.size() == 0);
        CHECK(empty.begin() == empty.end());
        
        // Bounds checking on empty view
        CHECK_NOTHROW(empty[0]);  // Should return nullptr or first element
    }
    
    TEST_CASE("different types") {
        // Test with various types
        uint8_t bytes[] = {0xFF, 0x00};
        array_view<uint8_t> byte_view(bytes, 2);
        CHECK(byte_view[0] == 0xFF);
        
        float floats[] = {1.0f, 2.0f, 3.0f};
        array_view<float> float_view(floats, 3);
        CHECK(float_view[1] == 2.0f);
    }
    
    TEST_CASE("view of view data") {
        int data[] = {1, 2, 3, 4, 5};
        array_view<const int> first_view(data, 5);
        
        // Create view from first view's data
        const int* raw_ptr = first_view.begin();
        array_view<const int> second_view(raw_ptr, first_view.size());
        
        CHECK(second_view[0] == first_view[0]);
        CHECK(second_view.size() == first_view.size());
    }
} 