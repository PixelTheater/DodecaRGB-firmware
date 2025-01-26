#include "settings.h"
#include "preset.h"

namespace Animation {

void Settings::resetToInitial(const std::string& name) {
    auto it = _definitions.find(name);
    if (it != _definitions.end()) {
        if (it->second.type == ParamType::Instance) {
            // Create a shared_ptr that doesn't delete the instance (since it's owned by ParamDefinition)
            _active_instances[name] = std::shared_ptr<void>(
                const_cast<void*>(it->second.initial_instance),
                [](void*) {} // Empty deleter
            );
        } else {
            _active_values[name] = it->second.initial_value;
        }
    }
}

} // namespace Animation 