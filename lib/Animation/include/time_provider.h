#pragma once

namespace Animation {
    class TimeProvider {
    public:
        virtual ~TimeProvider() = default;
        virtual unsigned long millis() const = 0;
    };

    class TestTimeProvider : public TimeProvider {
    public:
        unsigned long millis() const override {
            return _current_time;
        }
        
        void advance(unsigned long ms) {
            _current_time += ms;
        }
        
    private:
        unsigned long _current_time = 0;
    };
} // namespace Animation 