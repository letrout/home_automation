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
#include <map>
#include "fh_dotstar.h"
#include "fh_homesec.h"
#include "fh_sensors.h"
#include "fh_mqtt.h"

extern FhAmbientLight ambientLight;
extern float prim_temp_f, prim_hum;  // primary temp and humidity measurements
#ifdef FH_SUB_PEPPERS
extern uint8_t peppers[];
#endif
#ifdef SENSIRIONI2CSCD4X_H
extern FhScd40 scd4x;
#endif
#ifdef ADAFRUIT_SGP30_H
extern FhSgp30 sgp30;
#endif
#ifdef FH_HOMESEC_H
extern std::map<const char*, OwensDoor, char_cmp> owensDoors;
#endif
#ifdef ADAFRUIT_PM25AQI_H
extern FhPm25Aqi pm25Aqi;
#endif

FhDotstar pixels(NUM_DOTSTAR, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLOCK, DOTSTAR_BRG);

void FhDotstar::setup(uint8_t brightness) {
    begin(); // Initialize pins for output
    show();  // Turn all LEDs off ASAP
    brightness_ = brightness;
    setBrightness(brightness_);
    return;
}

uint8_t FhDotstar::ambientAdjust(bool set_brightness, uint8_t min, uint8_t max) {
    uint8_t adjusted;
    // FIXME: this full-range linear mapping is unsatisfying (LEDs too dark in low ambient light)
    adjusted = map(ambientLight.last_ambient_light(), 0, 8192, min, max);
    if (set_brightness) {
        brightness_ = adjusted;
        setBrightness(brightness_);
    }
    return adjusted;
}

uint8_t FhDotstar::setMode(byte mode, bool ambient_adjust) {
    mode_ = mode;
    switch(mode_) {
        case DOTSTAR_MODE_SLEEP:
            //setBrightness(0);
            clear();
            if (ambient_adjust) {
                setBrightness(ambientAdjust(true, 5, 100));
            }
#ifdef FH_HOMESEC_H
            if (owensDoors.at("library-front").is_open()) {
                setPixelColor(0, Color(0, 255, 0));
            }
            if (owensDoors.at("garage-side").is_open()) {
                setPixelColor(1, Color(0, 255, 0));
            }
            if (owensDoors.at("kitchen-deck").is_open()) {
                setPixelColor(2, Color(0, 255, 0));
            }
            if (owensDoors.at("garage-main").is_open()) {
                setPixelColor(3, Color(0, 255, 0));
            }
            if (owensDoors.at("mud-back").is_open()) {
                setPixelColor(4, Color(0, 255, 0));
            }
#endif
            show();
            break;
        case DOTSTAR_MODE_RAINBOW:
            if (ambient_adjust) {
                ambientAdjust(true, 10, 100);
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
                ambientAdjust(true, 5, 100);
            } else {
                // need something here
            }
#ifdef FH_SUB_PEPPERS
            uint16_t pepper_hues[PEPPER_PLANTS];
            for (int i=0; i < PEPPER_PLANTS; i++) {
                pepper_hues[i] = map(peppers[i], 0, 100, 26000, 0); // 0=red, 100=blue
                Serial.print("wet_pct: ");
                Serial.print(peppers[i]);
                Serial.print(", hue: ");
                Serial.println(pepper_hues[i]);
            }
            setPixelColor(0, gamma32(ColorHSV(pepper_hues[3])));
            setPixelColor(1, gamma32(ColorHSV(pepper_hues[2])));
            setPixelColor(3, gamma32(ColorHSV(pepper_hues[1])));
            setPixelColor(4, gamma32(ColorHSV(pepper_hues[0])));
            /*
            setPixelColor(0, gamma32(ColorHSV(pepper_hues[3], 255, brightness_)));
            setPixelColor(1, gamma32(ColorHSV(pepper_hues[2], 255, brightness_)));
            setPixelColor(3, gamma32(ColorHSV(pepper_hues[1], 255, brightness_)));
            setPixelColor(4, gamma32(ColorHSV(pepper_hues[0], 255, brightness_)));
            */
#endif
            show();
            break;
        case DOTSTAR_MODE_ENV:
            // Set far left, right dotstars to temp, humidity
            uint16_t temp_hue, hum_hue;
            // FIXME: should map blue-red instead of green-red
            temp_hue = map(prim_temp_f, 60, 85, 26000, 0);  // 60=green, 85=red
            setPixelColor(0, gamma32(ColorHSV(temp_hue)));
            Serial.print("Temp pixel hue ");
            Serial.println(temp_hue);
            // FIXME: should map blue-red instead of green-red
            hum_hue = map(prim_hum, 10, 85, 0, 26000);  // 85=green, 10=red
            setPixelColor(4, gamma32(ColorHSV(hum_hue)));
            Serial.print("Humidity pixel hue ");
            Serial.println(hum_hue);
#ifdef ADAFRUIT_PM25AQI_H
            // Set middle left to AQI PM2.5
            uint16_t pm25_hue;
#endif
#ifdef SENSIRIONI2CSCD4X_H
            // Set middle dotstar hue by CO2 level
            uint16_t co2_hue;
            co2_hue = map(scd4x.last_co2_ppm(), 400, 4000, 0, 26000);  // 400ppm=green, 4000ppm=red
            // setPixelColor(2, gamma32(ColorHSV(co2_hue, 255, brightness_)));
            setPixelColor(2, gamma32(ColorHSV(co2_hue)));
            Serial.print("CO2 pixel hue ");
            Serial.println(co2_hue);
#endif
#ifdef ADAFRUIT_SGP30_H
            // Set the middle right dotstar hue by VOC
            uint16_t voc_hue;
            voc_hue = map(sgp30.last_eco2(), 0, 2000, 0, 26000);  // 0ppm=green, 2000ppm=red
            // setPixelColor(2, gamma32(ColorHSV(co2_hue, 255, brightness_)));
            setPixelColor(3, gamma32(ColorHSV(voc_hue)));
            Serial.print("VOC pixel hue ");
            Serial.println(voc_hue);
#endif
            show();
            break;
    }
    return 0;
}