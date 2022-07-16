#include "fh_sensors.h"

Adafruit_DPS310 dps;

uint8_t setup_dps310() {
  Serial.println("INSIDE setup_dps310()");
  uint8_t retval = 1;
  for (uint8_t i = 0; i++; i < 5 ) {
    Serial.printf("attempting DPS310 I2C %d...", i);
    if (dps.begin_I2C()) {
      Serial.println("connected to DPS310...");
      retval = 0;
      break;
    } else {
      Serial.println("Connect to DPS310 FAILED!");
      delay(100);
    }
    Serial.println("setup_dps310 finished");
  }
  if (retval == 0) {
    Serial.println("Setup DPS310...");
    dps.configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
    dps.configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);
  }
  return retval;
}


uint8_t read_dps310() {
  sensors_event_t t, p;
  setup_dps310();
  if (dps.pressureAvailable() && dps.temperatureAvailable()) {
    Serial.println("DPS310 new temp and pressure available...");
    if (dps.getEvents(&t, &p)) {
      Serial.printf("DPS310 temp %f C, %f kpa\n", t.temperature, p.pressure);
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