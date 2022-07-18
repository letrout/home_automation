#ifndef FH_SENSORS_H
#define FH_SENSORS_H

#include <Adafruit_DPS310.h>
#include "fh_globals.h"

#define TEMP_F(c) (c * 9 / 5) + 32
#define TEMP_C(f) (f - 32) * 9 / 5

/**
 * @brief Class to extend Adafruit_DPS310
 * 
 */
class FhDps310 : public Adafruit_DPS310 {
  private:
    float last_temp_f_;
    float last_press_hpa_;
    unsigned long last_read_ms_;

  public:
    FhDps310();
    const float & last_temp_f()     { return last_temp_f_; }
    const float & last_temp_c()     { return TEMP_C(last_temp_f_); }
    const float & last_press_hpa()  { return last_press_hpa_; }
    const unsigned long & last_read_ms()  { return last_read_ms_; }

    /**
     * @brief Initialize the dps310 object
     * 
     * @return uint8_t 0 on success
     */
    uint8_t setupDps310();

    /**
     * @brief Read values from the DPS310
     * 
     * @return uint8_t 0 on success, 1 on failure, 2 if new values not available
     */
    uint8_t readDps310();
};

#endif /* FH_SENSORS_H */