#include "PixelTheater/scene.h"
#include <iostream>

namespace PixelTheater {

const char* Scene::name() const {
    return _metadata ? _metadata->name : "unnamed";
}

const char* Scene::description() const {
    return _metadata ? _metadata->description : "";
}

void Scene::init_params() {
    config();  // Just call config() directly
}

void Scene::param(const std::string& name, const std::string& type,
                 const ParamValue& default_val, const std::string& flags) {
    _settings_storage.add_parameter_from_strings(name, type, default_val, flags);
}

// Convenience overloads
void Scene::param(const std::string& name, const std::string& type,
                 float default_val, const std::string& flags) {
    param(name, type, ParamValue(default_val), flags);
}

void Scene::param(const std::string& name, const std::string& type,
                 int default_val, const std::string& flags) {
    param(name, type, ParamValue(default_val), flags);
}

void Scene::param(const std::string& name, const std::string& type,
                 bool default_val, const std::string& flags) {
    param(name, type, ParamValue(default_val), flags);
}

} // namespace PixelTheater
