/**
 * @file fh_tft.cpp
 * @author Joel Luth (joel.luth@gmail.com)
 * @brief Classes to manage TFT behavior used on Adafruit Funhouse
 * @version 0.1
 * @date 2022-07-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "fh_tft.h"

// display!
Adafruit_ST7789 tft = FhTft(TFT_CS, TFT_DC, TFT_RESET);

// Funhouse ST7789 TFT display
void FhTft::setup(void) {
    init(240, 240);                // Initialize ST7789 screen
    pinMode(TFT_BACKLIGHT, OUTPUT);
    digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on

    fillScreen(BG_COLOR);
    setTextSize(2);
    setTextColor(ST77XX_YELLOW);
    setTextWrap(false);
    return;
}