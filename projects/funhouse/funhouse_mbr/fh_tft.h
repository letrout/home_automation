#ifndef FH_TFT_H
#define FH_TFT_H

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789

// FIXME: move to cpp file when removed from funhouse_mbr.ino
#define BG_COLOR ST77XX_BLACK

// Display "modes"
#define DISPLAY_MODE_SLEEP 0
#define DISPLAY_MODE_ALL_SENSORS 1
#define DISPLAY_MODE_ENVIRONMENTAL 2

#define TEXT_LINE_PXL(t) (t * 10) // line hieght in pixels, from test size (setTestSize())

class FhTft : public Adafruit_ST7789 {
  private:
    byte display_mode_;
    /**
     * @brief print display of sensors as they are discovered
     * 
     * @return uint8_t error code, 0 on success
     */
    uint8_t sensorSetup();

  public:
    FhTft(int8_t cs, int8_t dc, int8_t rst)
    : Adafruit_ST7789{cs, dc, rst}
    {
    }
    byte getDisplayMode() { return display_mode_; }
    uint8_t getCursorY() const { return cursor_y; }
    uint8_t getTextSize() const { return textsize_y; }
    /**
     * @brief set up the TFT display
     * 
     * @return 
     */
    void setup();
    /**
     * @brief Set the display mode
     * sets background color, backlight, etc
     * 
     * @return uint8_t 
     */
    uint8_t setDisplayMode(byte mode, bool fill = false);
    /**
    * @brief display sensor values on the TFT
     * 
     * @param fill - if true fill background (passthru to FhTft::setDsiplayMode)
     * 
     */
    void displaySensors(bool fill = false);
    /**
     * @brief display environment information
     * 
     * @param fill - if true fill background (passthru to FhTft::setDsiplayMode)
     */
    void displayEnvironment(bool fill = false);
    /**
     * @brief display door state information
     * 
     * @param fill 
     */
    void displayDoors(bool fill = false);
};

#endif /* FH_TFT_H */