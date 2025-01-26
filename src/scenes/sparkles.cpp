#include "animation.h"
#include "animations/sparkles.h"

void Sparkles::init(const AnimParams& params) {
    Animation::init(params);
    
    // Initialize parameters from AnimParams
    period = params.getInt("period", 580);
    base_palette = params.getPalette("base_palette", CRGBPalette16(CRGB::Blue));
    highlight_palette = params.getPalette("highlight_palette", CRGBPalette16(CRGB::White));
    
    // Generate local random seeds
    seed1 = random(4000);
    seed2 = random(5000);
    
    // Initialize state variables
    power_fade = 20;  // Set initial value
}

void Sparkles::tick() {
    uint8_t color_warble = (int)(sin8_C(millis()/(period/11)) / 16);
    color_mix = (int)(64 + sin8_C(millis()/(period)+seed1+color_warble) / 1.5);
    
    c = ColorFromPaletteExtended(base_palette, sin16_C(millis()/16+seed1*10), 255, LINEARBLEND);
    c2 = ColorFromPaletteExtended(highlight_palette, sin16_C((millis()/8+seed2*50)), 255, LINEARBLEND);
    blend1 = sin8_C(millis()/(period*4.2)+seed1)/1.5+32;
    blend2 = sin8_C(millis()/(period*3.5)+seed2)/2+32;
    
    for (int n = 0; n < num_sides; n++) {
        num_picks = map(power_fade, 1, 40, 30, 5);
        for (int m = 0; m < num_picks; m++) {
            if (random8(128) < color_mix) {
                int r1 = random(n*leds_per_side, (n+1)*leds_per_side);        
                nblend(leds[r1], c, map(blend1, 0, 255, 1, 7));
            }
            if (random8(128) < (256-color_mix)) {
                int r2 = random(n*leds_per_side, (n+1)*leds_per_side);
                nblend(leds[r2], c2, map(blend2, 0, 255, 1, 10));        
            }
        }
    }
    
    int power = calculate_unscaled_power_mW(leds, numLeds());
    power_fade = (power_fade * 19 + max(map(power,8000,20000,1,40),1))/20;
    
    for (int i = 0; i < numLeds(); i++) {
        int v = leds[i].getAverageLight();
        if (v > random8(power_fade/2)) {
            leds[i].fadeToBlackBy(random8(power_fade));
        }
    }
}

String Sparkles::getStatus() const {
    output.printf("mix=%d/%d fade=%d picks=%d\n", 
        color_mix * 100 / 256, (256 - color_mix) * 100 / 256, 
        power_fade, num_picks);
    
    output.print(getAnsiColorString(c));
    output.printf(" color1: %02X%02X%02X (%s) blend1: %d%%\n",
        c.r, c.g, c.b, getClosestColorName(c).c_str(), 
        blend1 * 100 / 256);
    
    output.print(getAnsiColorString(c2));
    output.printf(" color2: %02X%02X%02X (%s) blend2: %d%%",
        c2.r, c2.g, c2.b, getClosestColorName(c2).c_str(), 
        blend2 * 100 / 256);
    
    return output.get();
}