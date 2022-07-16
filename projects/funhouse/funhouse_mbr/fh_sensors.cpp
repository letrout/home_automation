#include "fh_sensors.h"

// sensors objects
FhDps310 dps;

// sensor values
// DPS310
sensors_event_t dps_temp, dps_pressure;

// DPS310
FhDps310::FhDps310(void) {
}

uint8_t FhDps310::setup_dps310(void) {
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

uint8_t FhDps310::read_dps310(void) {
  sensors_event_t t, p;
  if (pressureAvailable() && temperatureAvailable()) {
    if (getEvents(&t, &p)) {
      _last_temp_c = t.temperature;
      _last_press_hpa = p.pressure;
      _last_read_ms = millis();
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
    return 2;
  }
}