#ifndef FH_TFT_H
#define FH_TFT_H

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789

// FIXME: move to cpp file when removed from funhouse_mbr.ino
#define BG_COLOR ST77XX_BLACK

class FhTft : public Adafruit_ST7789 {
  private:

  public:
    FhTft(int8_t cs, int8_t dc, int8_t rst)
    : Adafruit_ST7789{cs, dc, rst}
    {
    }
    /**
     * @brief set up the TFT display
     * 
     * @return 
     */
    void setup();
};

#endif /* FH_TFT_H */