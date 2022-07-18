#include "fh_sensors.h"

// sensors objects
FhDps310 dps;

// DPS310
FhDps310::FhDps310(void) {
}

uint8_t FhDps310::setupDps310(void) {
  uint8_t i;
  uint8_t retval = 1;
  for (i = 1; i++; i <= 5 ) {
    if (begin_I2C()) {
      retval = 0;
      break;
    } else {
      Serial.println("Connect to DPS310 FAILED!");
      delay(100);
    }
  }
  if (retval == 0) {
    configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
    configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);
  }
  return retval;
}

uint8_t FhDps310::readDps310(void) {
  sensors_event_t t, p;
  if (pressureAvailable() && temperatureAvailable()) {
    if (getEvents(&t, &p)) {
      last_temp_f_ = TEMP_F(t.temperature);
      last_press_hpa_ = p.pressure;
      last_read_ms_ = millis();
      return 0;
    } else {
      Serial.println("DPS310 read failed!");
      return 1;
    }
  } else {
    Serial.println("DPS310 temp or pressure not available");
    return 2;
  }
}
// End DPS310