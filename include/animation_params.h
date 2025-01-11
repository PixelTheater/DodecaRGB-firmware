#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include <map>
#include <string>

struct AnimParams {
    // Custom parameters with defaults
    std::map<String, float> custom_floats;
    std::map<String, int> custom_ints;
    std::map<String, CRGBPalette16> custom_palettes;
    
    float getFloat(const String& key, float default_value = 0.0f) const {
        auto it = custom_floats.find(key);
        return it != custom_floats.end() ? it->second : default_value;
    }

    int getInt(const String& key, int default_value = 0) const {
        auto it = custom_ints.find(key);
        return it != custom_ints.end() ? it->second : default_value;
    }
    
    CRGBPalette16 getPalette(const String& key, const CRGBPalette16& default_value = CRGBPalette16(CRGB::Black)) const {
        auto it = custom_palettes.find(key);
        return it != custom_palettes.end() ? it->second : default_value;
    }
    
    void setFloat(const String& key, float value) { custom_floats[key] = value; }
    void setInt(const String& key, int value) { custom_ints[key] = value; } 
    void setPalette(const String& key, const CRGBPalette16& value) { custom_palettes[key] = value; }
};
