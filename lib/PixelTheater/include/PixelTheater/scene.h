#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include "settings.h"
#include "settings_proxy.h"
#include "params/param_def.h"
#include "params/param_value.h"
#include "model/model.h"
#include "platform/platform.h"
#include "core/imodel.h"
#include "core/iled_buffer.h"
#include <cstdarg>

// Forward declare to avoid circular dependency
namespace PixelTheater {
    class Scene;
    class Theater;
    struct SceneParameterSchema;
}

// Now include the param_schema.h file
#include "params/param_schema.h"

// Forward declare test fixture OUTSIDE namespace
// struct SceneHelperFixture; // No longer needed
template<typename SceneType> struct NewSceneFixture; // Forward declare templated fixture

namespace PixelTheater {

    // Forward declarations within namespace
    class Scene;
    class Theater;
    struct SceneParameterSchema;

    namespace ParamSchema {
        SceneParameterSchema generate_schema(const Scene& scene);
    }

    // --- Define LedsProxy struct BEFORE Scene uses it --- 
    struct LedsProxy {
    private:
        ILedBuffer* _buffer_ptr;
        friend class Scene;
        // Allow construction with nullptr
        LedsProxy(ILedBuffer* buffer = nullptr) : _buffer_ptr(buffer) {}

    public:
        // Array-like access operator
        CRGB& operator[](size_t i) {
            if (!_buffer_ptr) { 
                static CRGB dummyLed = CRGB::Black; 
                return dummyLed; 
            } 
            return _buffer_ptr->led(i); // Delegates bounds check to buffer
        }
        const CRGB& operator[](size_t i) const {
            if (!_buffer_ptr) { 
                static const CRGB dummyLed = CRGB::Black; 
                return dummyLed; 
            } 
            return _buffer_ptr->led(i); // Delegates bounds check to buffer
        }

        // Size method
        size_t size() const {
            return _buffer_ptr ? _buffer_ptr->ledCount() : 0;
        }

        // Allow assignment to update the internal pointer after Scene::connect
        LedsProxy& operator=(const LedsProxy& other) = default;

        // Note: Iteration (begin/end) not directly supported by this simple proxy.
        // Use scene.ledCount() and scene.led(i) for loops.
    };

    // Scene - A single animation running on a Stage, with its own parameters and state
    //  - Serves as a base class that can be extended by users of the library for their own scenes
    //  - lifecycle management (setup, tick, reset, etc)
    //  - Manages its own parameters (via Settings)

    /* see creating_animations.md for more information */

    class Scene {
    public:
        // Explicit Default Constructor
        Scene()
            : _settings_storage()
            , settings(_settings_storage)
            , leds(nullptr) 
            , _name("Unnamed Scene")
            , _description("")
            , _version("1.0")
            , _author("")
            , _tick_count(0) // Initialize here too
        {
             init_params(); // Call param initialization
        }

        // Constructor with params (existing)
        Scene(const ParamDef* params, size_t param_count) // Removed default args to avoid ambiguity
            : _settings_storage()
            , settings(_settings_storage)
            , leds(nullptr) 
            , _name("Unnamed Scene")
            , _description("")
            , _version("1.0")
            , _author("")
            , _tick_count(0) 
        {
            if (params) {
                _settings_storage = Settings(params, param_count);
            } else {
                init_params();
            }
        }

        virtual ~Scene() = default;

        // Prevent copying
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;

        /**
         * Initialize scene state
         * Called once when scene becomes active
         */
        virtual void setup() = 0;

        /**
         * Update animation state
         * Called every frame (50fps+)
         */
        virtual void tick() {
            _tick_count++;
        }

        /**
         * Reset scene to initial state
         * Optional override
         */
        virtual void reset() {
            _tick_count = 0; // Ensure reset happens first
            settings.reset_all();
        }

        /**
         * Set scene name
         * @param name Scene name
         */
        void set_name(const std::string& name) {
            _name = name;
        }

        /**
         * Set scene description
         * @param description Scene description
         */
        void set_description(const std::string& description) {
            _description = description;
        }

        /**
         * Set scene version
         * @param version Scene version
         */
        void set_version(const std::string& version) {
            _version = version;
        }

        /**
         * Set scene author
         * @param author Scene author
         */
        void set_author(const std::string& author) {
            _author = author;
        }

        /**
         * Get scene name
         * @return Name of the scene
         */
        const std::string& name() const {
            return _name;
        }

        /**
         * Get scene description
         * @return Description of the scene
         */
        const std::string& description() const {
            return _description;
        }

        /**
         * Get scene version
         * @return Version of the scene
         */
        const std::string& version() const {
            return _version;
        }

        /**
         * Get scene author
         * @return Author of the scene
         */
        const std::string& author() const {
            return _author;
        }

        /**
         * Get number of ticks processed
         * @return Tick count
         */
        size_t tick_count() const { return _tick_count; }

        /**
         * Get all parameter names defined for this scene
         * @return Vector of parameter names
         */
        std::vector<std::string> get_parameter_names() const {
            return _settings_storage.get_parameter_names();
        }

        /**
         * Get metadata for a specific parameter
         * @param name Parameter name
         * @return Parameter metadata
         */
        const ParamDef& get_parameter_metadata(const std::string& name) const {
            return _settings_storage.get_metadata(name);
        }

        /**
         * Check if a parameter exists
         * @param name Parameter name
         * @return True if the parameter exists, false otherwise
         */
        bool has_parameter(const std::string& name) const {
            return _settings_storage.has_parameter(name);
        }

        /**
         * Get the type of a parameter
         * @param name Parameter name
         * @return Parameter type
         */
        ParamType get_parameter_type(const std::string& name) const {
            return _settings_storage.get_type(name);
        }

        /**
         * Get parameter schema for this scene
         * @return Parameter schema
         */
        SceneParameterSchema parameter_schema() const {
            return ParamSchema::generate_schema(*this);
        }

        /**
         * Get parameter schema as JSON string
         * @return JSON string
         */
        std::string parameter_schema_json() const {
            return parameter_schema().to_json();
        }

        // --- Scene Helper Methods --- 
        size_t ledCount() const { return leds_ptr ? leds_ptr->ledCount() : 0; }
        CRGB& led(size_t index) {
            if (!leds_ptr) { 
                static CRGB dummyLed = CRGB::Black; 
                logError("Scene::led() called before leds connected"); 
                return dummyLed; 
            } 
            return leds_ptr->led(index); 
        }
        const CRGB& led(size_t index) const {
            if (!leds_ptr) { 
                static const CRGB dummyLed = CRGB::Black; 
                logError("Scene::led() const called before leds connected");
                return dummyLed; 
            } 
            return leds_ptr->led(index);
        }
        float deltaTime() const { return platform_ptr ? platform_ptr->deltaTime() : 0.0f; }
        uint32_t millis() const { return platform_ptr ? platform_ptr->millis() : 0; }
        uint8_t random8() { return platform_ptr ? platform_ptr->random8() : 0; }
        uint16_t random16() { return platform_ptr ? platform_ptr->random16() : 0; }
        uint32_t random(uint32_t max = 0) { return platform_ptr ? platform_ptr->random(max) : 0; }
        uint32_t random(uint32_t min, uint32_t max) { return platform_ptr ? platform_ptr->random(min, max) : 0; }
        
        // --- ADDED MISSING DELEGATION --- 
        float randomFloat() { return platform_ptr ? platform_ptr->randomFloat() : 0.0f; } // 0.0 to 1.0
        // --- END ADDED --- 
        
        float randomFloat(float max) { return platform_ptr ? platform_ptr->randomFloat(max) : 0.0f; } // 0.0 to max
        float randomFloat(float min, float max) { return platform_ptr ? platform_ptr->randomFloat(min, max) : 0.0f; } // min to max

        // Model Geometry Access (NEW)
        const IModel& model() const; // Implementation in .cpp

        // REMOVED direct geometry helpers:
        // size_t faceCount() const { /* ... */ } 
        // const Point& point(size_t index) const { /* ... */ }
        // const Face& face(size_t index) const { /* ... */ }

        // Timing Utilities Helpers
        // ... deltaTime(), millis() ...

        // Math/Random Utilities Helpers
        // ... random*() ...

        // Logging Utilities Helpers (Variadic)
        // Non-const versions
        void logInfo(const char* format, ...) {
            if (!platform_ptr) return;
            char buffer[256]; // Static buffer, adjust size if needed
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            platform_ptr->logInfo(buffer); // Pass the formatted buffer
        }
        void logWarning(const char* format, ...) {
            if (!platform_ptr) return;
            char buffer[256];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            platform_ptr->logWarning(buffer); // Pass the formatted buffer
        }
        void logError(const char* format, ...) {
            if (!platform_ptr) return;
            char buffer[256];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            platform_ptr->logError(buffer); // Pass the formatted buffer
        }
        // Const versions
        void logInfo(const char* format, ...) const {
            if (!platform_ptr) return;
            char buffer[256];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            platform_ptr->logInfo(buffer);
        }
        void logWarning(const char* format, ...) const {
            if (!platform_ptr) return;
            char buffer[256];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            platform_ptr->logWarning(buffer); 
        }
        void logError(const char* format, ...) const {
            if (!platform_ptr) return;
            char buffer[256];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            platform_ptr->logError(buffer);
        }
        
        // --- End Scene Helper Methods --- 

        // --- Public Member Declarations --- 
        Settings _settings_storage;
        SettingsProxy settings;
        LedsProxy leds;
        // Metadata members (matching initializer list order)
        std::string _name;
        std::string _description;
        std::string _version;
        std::string _author;

    protected:
        // Pointers to the core components provided by Theater
        IModel* model_ptr = nullptr;
        ILedBuffer* leds_ptr = nullptr;
        Platform* platform_ptr = nullptr;
        // Initialized tick count (matches initializer list order)
        size_t _tick_count{0}; 

        /**
         * Define a parameter with a string type and default value
         * @param name Parameter name
         * @param type Parameter type as string (e.g., "ratio", "count", "switch")
         * @param default_val Default value
         * @param flags Optional flags (e.g., "clamp", "wrap")
         * @param description Optional description
         */
        void param(const std::string& name, const std::string& type,
                  const ParamValue& default_val, const std::string& flags = "",
                  const std::string& description = "") {
            _settings_storage.add_parameter_from_strings(name, type, default_val, flags, description);
        }

        /**
         * Configure scene parameters
         * Called during initialization if no generated params
         */
        virtual void config() {}

        /**
         * Define a parameter with a float default value
         */
        void param(const std::string& name, const std::string& type,
                  float default_val, const std::string& flags = "",
                  const std::string& description = "") {
            param(name, type, ParamValue(default_val), flags, description);
        }
        
        /**
         * Define a parameter with an int default value
         */
        void param(const std::string& name, const std::string& type,
                  int default_val, const std::string& flags = "",
                  const std::string& description = "") {
            param(name, type, ParamValue(default_val), flags, description);
        }
        
        /**
         * Define a parameter with a bool default value
         */
        void param(const std::string& name, const std::string& type,
                  bool default_val, const std::string& flags = "",
                  const std::string& description = "") {
            param(name, type, ParamValue(default_val), flags, description);
        }

        /**
         * Define a parameter with an integer range
         * @param name Parameter name
         * @param type Parameter type (usually "count")
         * @param min Minimum value
         * @param max Maximum value
         * @param default_val Default value
         * @param flags Optional flags
         * @param description Optional description
         */
        void param(const std::string& name, const std::string& type,
                  int min, int max, int default_val, const std::string& flags = "",
                  const std::string& description = "") {
            if (type == "count") {
                _settings_storage.add_count_parameter(name, min, max, default_val, flags, description);
            } else {
                param(name, type, default_val, flags, description);
            }
        }
        
        /**
         * Define a parameter with a float range
         * @param name Parameter name
         * @param type Parameter type (usually "range")
         * @param min Minimum value
         * @param max Maximum value
         * @param default_val Default value
         * @param flags Optional flags
         * @param description Optional description
         */
        void param(const std::string& name, const std::string& type, float min, float max, float default_val, const std::string& flags = "", const std::string& description = "") {
            if (type == "range") _settings_storage.add_range_parameter(name, min, max, default_val, flags, description);
            else param(name, type, default_val, flags, description);
        }

        // Metadata Definition Method
        void meta(const std::string& key, const std::string& value) {
            // TODO: Implement proper metadata storage if needed beyond settings
            // For now, maybe store in settings if key doesn't conflict?
             if (!_settings_storage.has_parameter(key)) {
                 // Use param() to store simple string metadata? Might be confusing.
                 // param(key, "string", value, "readonly", "Metadata"); 
                 // Or add a dedicated metadata map?
                 // std::map<std::string, std::string> _metadata;
                 // _metadata[key] = value;
                 
                 // TEMPORARY: Use set_name etc. if key matches known metadata
                 if (key == "title" || key == "name") set_name(value);
                 else if (key == "description") set_description(value);
                 else if (key == "version") set_version(value);
                 else if (key == "author") set_author(value);
                 // else Log::warning("Unknown metadata key: %s", key.c_str());
             }
        }

        // Connect method - protected again for testing interface access
        void connect(IModel& model_ref, ILedBuffer& leds_ref, Platform& platform_ref);

    private:
        friend class Theater; 
        // Allow the specific test fixture template to access connect
        template<typename SceneType> friend struct ::NewSceneFixture; 
        
        void init_params() { config(); }
    }; // End class Scene

} // namespace PixelTheater 