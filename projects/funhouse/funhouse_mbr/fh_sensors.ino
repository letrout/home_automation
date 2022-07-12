#include "fh_sensors.h"

uint8_t setup_dps310() {
  uint8_t retval = 1;
  for (uint8_t i = 0; i++; i < 5 ) {
    if (dps.begin_I2C()) {
      retval = 0;
      break;
    } else {
      delay(100);
    }
  }
  if (retval = 0) {
    dps.configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
    dps.configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);
  }
  return retval;
}


uint8_t read_dps310() {
  // FIXME: Does dps.getEvents() have a return value to check?
  dps.getEvents(&dps_temp, &dps_pressure);
  return 0;
}