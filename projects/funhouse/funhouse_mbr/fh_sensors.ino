#include "fh_sensors.h"

Adafruit_DPS310 dps;

uint8_t setup_dps310() {
  uint8_t i;
  uint8_t retval = 1;
  for (i = 1; i++; i <= 5 ) {
    if (dps.begin_I2C()) {
      retval = 0;
      break;
    } else {
      Serial.println("Connect to DPS310 FAILED!");
      delay(100);
    }
  }
  if (retval == 0) {
    dps.configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
    dps.configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);
  }
  return retval;
}


uint8_t read_dps310() {
  sensors_event_t t, p;
  if (dps.pressureAvailable() && dps.temperatureAvailable()) {
    if (dps.getEvents(&t, &p)) {
      dps_temp = t;
      dps_pressure = p;
      return 0;
    } else {
      Serial.println("DPS310 read failed!");
      return 1;
    }
  } else {
    Serial.println("DPS310 temp or pressure not available");
    return 1;
  }
}