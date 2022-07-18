#ifndef FH_SENSORS_H
#define FH_SENSORS_H

#include <Adafruit_DPS310.h>
#include <Adafruit_AHTX0.h>
#include "fh_globals.h"

#define TEMP_F(c) (c * 9 / 5) + 32
#define TEMP_C(f) (f - 32) * 5 / 9

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
    float last_temp_f() const { return last_temp_f_; }
    float last_temp_c() const { return TEMP_C(last_temp_f_); }
    float last_press_hpa() const { return last_press_hpa_; }
    unsigned long last_read_ms() const { return last_read_ms_; }

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

/**
 * @brief Class to extend Adafruit_AHTX0
 * 
 */
class FhAht20 : public Adafruit_AHTX0 {
  private:
    float last_temp_f_;
    float last_hum_pct_;
    unsigned long last_read_ms_;

  public:
    FhAht20();
    float last_temp_f() const { return last_temp_f_; }
    float last_temp_c() const { return TEMP_C(last_temp_f_); }
    float last_hum_pct() const { return last_hum_pct_; }
    unsigned long last_read_ms() const { return last_read_ms_; }

    uint8_t readAht20();
};

#endif /* FH_SENSORS_H */