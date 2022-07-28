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

FhDotstar pixels(NUM_DOTSTAR, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLOCK, DOTSTAR_BRG);

void FhDotstar::setup(void) {
    begin(); // Initialize pins for output
    show();  // Turn all LEDs off ASAP
    setBrightness(255);
    return;
} 