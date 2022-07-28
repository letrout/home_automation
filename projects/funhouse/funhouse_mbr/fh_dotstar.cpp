/**
 * @file fh_dotstar.cpp
 * @author Joel Luth (joel.luth@gmail.com)
 * @brief Classes to manage dotstar (neopixel) behavior used on Adafruit Funhouse
 * @version 0.1
 * @date 2022-07-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "fh_dotstar.h"
#include "fh_sensors.h"

extern FhAmbientLight ambientLight;

FhDotstar pixels(NUM_DOTSTAR, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLOCK, DOTSTAR_BRG);

void FhDotstar::setup(uint8_t brightness) {
    begin(); // Initialize pins for output
    show();  // Turn all LEDs off ASAP
    brightness_ = brightness;
    setBrightness(brightness_);
    return;
}

uint8_t FhDotstar::ambientAdjusted(void) {
    // FIXME: this full-range linear mapping is unsatisfying (LEDs too dark in low ambient light)
    return map(ambientLight.last_ambient_light(), 0, 8192, 0, 255);
}

uint8_t FhDotstar::setMode(byte mode, bool ambient_adjust) {
    mode_ = mode;
    switch(mode_) {
        case DOTSTAR_MODE_SLEEP:
            setBrightness(0);
            show();
            break;
        case DOTSTAR_MODE_RAINBOW:
            for (uint16_t i=0; i<numPixels(); i++) { // For each pixel in strip...
                uint16_t pixel_hue = first_pixel_hue_ + (i * 65536L / numPixels());
                setPixelColor(i, gamma32(ColorHSV(pixel_hue)));
            }
            first_pixel_hue_ += 256;
            show();
            break;
    }
    return 0;
}