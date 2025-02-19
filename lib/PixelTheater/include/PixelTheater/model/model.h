#pragma once
#include <vector>
#include <string>
#include "face.h"
#include "../led.h"
#include "../limits.h"

namespace PixelTheater {

// Model provides runtime access to:
// 1. LEDs and their properties (position, neighbors)
// 2. Faces and their regions (center, rings, edges)
// 3. Model metadata (name, version, description)
template<typename ModelDef>
class Model {

private:
    std::string _name;
    std::string _version;
    std::string _description;

};

} // namespace PixelTheater 