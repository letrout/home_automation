#ifndef FH_SENSORS_H
#define FH_SENSORS_H

#include <Adafruit_DPS310.h>
#include "fh_globals.h"

// sensor values
// DPS310
extern sensors_event_t dps_temp, dps_pressure;

/**
 * @brief Class to extend Adafruit_DPS310
 * 
 */
class FhDps310 : public Adafruit_DPS310 {
  private:
    float _last_temp_c;
    float _last_press_hpa;
    unsigned long _last_read_ms;

  public:
    FhDps310();
    const float & last_temp_c()     { return _last_temp_c; }
    const float & last_press_hpa()  { return _last_press_hpa; }
    const unsigned long & last_read_ms()  { return _last_read_ms; }

    /**
     * @brief Initialize the dps310 object
     * 
     * @return uint8_t 0 on success
     */
    uint8_t setup_dps310();

    /**
     * @brief Read values from the DPS310
     * 
     * @return uint8_t 0 on success, 1 on failure, 2 if new values not available
     */
    uint8_t read_dps310();
};


/**
 * @brief Initialize the dps310 object
 * 
 * @return uint8_t 0 on success
 */
uint8_t setup_dps310(void);


/**
 * @brief Read values from the DPS310
 * 
 * @return uint8_t 0 on success, 1 on failure, 2 if new values not available
 */
uint8_t read_dps310(void);

#endif /* FH_SENSORS_H */