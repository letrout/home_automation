#ifndef FH_SENSORS_H
#define FH_SENSORS_H

#include <Adafruit_DPS310.h>
#include "fh_globals.h"

// sensors objects
Adafruit_DPS310 dps;

// sensor values
// DPS310
sensors_event_t dps_temp, dps_pressure;


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