#ifndef FH_DOTSTAR_H
#define FH_DOTSTAR_H

#include <Adafruit_DotStar.h>

// FIXME: funhouse_mbr.h only used for PEPPER_PLANTS, change when that define is moved
#include "funhouse_mbr.h"

#define NUM_DOTSTAR 5
#define DOTSTAR_MODE_SLEEP 0
#define DOTSTAR_MODE_RAINBOW 1
#define DOTSTAR_MODE_PLANTS 2

class FhDotstar : public Adafruit_DotStar {
  private:
    byte mode_;
    uint8_t brightness_ = 128;
    uint16_t first_pixel_hue_ = 0;
    /**
     * @brief Set LED brightness adjusted for the ambient light
     * 
     * @return nothing
     */
    void ambientAdjust();

  public:
    FhDotstar(uint16_t n, uint8_t d, uint8_t c, uint8_t o = (uint8_t) 9U)
    : Adafruit_DotStar{n, d, c, o}
    {
    }
    byte getMode() { return mode_; }
    uint8_t getBrightness() { return brightness_; }
    /**
     * @brief Set the dotstar mode
     * 
     * @param mode - the mode to set
     * @param ambient_adjust - if true, use ambient light to adjust LED brightness
     * 
     * @return uint8_t 
     */
    uint8_t setMode(byte mode, bool ambient_adjust = true);
    /**
     * @brief setup the dotstars
     * 
     * @return nothing
     */
    void setup(uint8_t brightness = 255);
};

#endif /* FH_DOTSTAR_H */