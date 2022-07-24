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
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RESET);