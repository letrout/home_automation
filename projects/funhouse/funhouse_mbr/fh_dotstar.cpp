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
extern uint8_t peppers[];

FhDotstar pixels(NUM_DOTSTAR, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLOCK, DOTSTAR_BRG);

void FhDotstar::setup(uint8_t brightness) {
    begin(); // Initialize pins for output
    show();  // Turn all LEDs off ASAP
    brightness_ = brightness;
    setBrightness(brightness_);
    return;
}

void FhDotstar::ambientAdjust(void) {
    // FIXME: this full-range linear mapping is unsatisfying (LEDs too dark in low ambient light)
    brightness_ = map(ambientLight.last_ambient_light(), 0, 8192, 0, 255);
    setBrightness(brightness_);
    return;
}

uint8_t FhDotstar::setMode(byte mode, bool ambient_adjust) {
    mode_ = mode;
    switch(mode_) {
        case DOTSTAR_MODE_SLEEP:
            setBrightness(0);
            show();
            break;
        case DOTSTAR_MODE_RAINBOW:
            if (ambient_adjust) {
                ambientAdjust();
            } else {
                // need something here
            }
            for (uint16_t i=0; i<numPixels(); i++) { // For each pixel in strip...
                uint16_t pixel_hue = first_pixel_hue_ + (i * 65536L / numPixels());
                setPixelColor(i, gamma32(ColorHSV(pixel_hue)));
            }
            first_pixel_hue_ += 256;
            show();
            break;
        case DOTSTAR_MODE_PLANTS:
            if (ambient_adjust) {
                ambientAdjust();
            } else {
                // need something here
            }
            uint16_t pepper_hues[PEPPER_PLANTS];
            for (int i=0; i < PEPPER_PLANTS; i++) {
                pepper_hues[i] = map(peppers[i], 0, 100, 26000, 0); // 0=red, 100=blue
                Serial.print("wet_pct: ");
                Serial.print(peppers[i]);
                Serial.print(", hue: ");
                Serial.println(pepper_hues[i]);
            }
            setPixelColor(0, gamma32(ColorHSV(pepper_hues[3], 255, brightness_)));
            setPixelColor(1, gamma32(ColorHSV(pepper_hues[2], 255, brightness_)));
            setPixelColor(3, gamma32(ColorHSV(pepper_hues[1], 255, brightness_)));
            setPixelColor(4, gamma32(ColorHSV(pepper_hues[0], 255, brightness_)));
            show();
            break;
    }
    return 0;
}