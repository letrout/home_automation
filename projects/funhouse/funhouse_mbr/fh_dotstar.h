#ifndef FH_DOTSTAR_H
#define FH_DOTSTAR_H

#include <Adafruit_DotStar.h>

#define NUM_DOTSTAR 5
#define DOTSTAR_MODE_SLEEP 0

class FhDotstar : public Adafruit_DotStar {
  private:
    byte display_mode_;

  public:
    FhDotstar(uint16_t n, uint8_t d, uint8_t c, uint8_t o = (uint8_t) 9U)
    : Adafruit_DotStar{n, d, c, o}
    {
    }
    byte getDisplayMode() { return display_mode_; }
    /**
     * @brief setup the dotstars
     * 
     * @return nothing
     */
    void setup();
};

#endif /* FH_DOTSTAR_H */