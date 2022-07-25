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
    return;
}

uint8_t FhTft::setDisplayMode(byte mode) {
    uint8_t retval = 0;
    display_mode_ = mode;
    switch(display_mode_) {
        // TODO: make the sleep mode the default?
        case DISPLAY_MODE_SLEEP :
            digitalWrite(TFT_BACKLIGHT, LOW); // Backlight off
            break;
        case DISPLAY_MODE_ALL_SENSORS :
            text_size_ = 2;
            cursor_y_ = 0;
            digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on
            fillScreen(BG_COLOR);
            setTextSize(text_size_);
            setTextColor(ST77XX_YELLOW);
            setTextWrap(false);
            break;
        default:
            retval = 1;
    }
    return retval;
}

size_t FhTft::println(const char* t) {
    size_t retval;
    retval = Adafruit_ST7789::println(t);
    cursor_y_ += TEXT_LINE_PXL(text_size_);
    setCursor(0, cursor_y_);
    return retval;
}