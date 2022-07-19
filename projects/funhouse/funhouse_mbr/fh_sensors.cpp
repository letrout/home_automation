#include "fh_sensors.h"

// sensors objects
FhDps310 dps;
FhAht20 aht;
#ifdef ADAFRUIT_SGP30_H
FhSgp30 sgp30;
#endif
#ifdef ADAFRUIT_SHT4x_H
FhSht40 sht4x = FhSht40();
#endif

uint32_t getAbsoluteHumidity(float temp_c, float hum_pct) {
  // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
  const float absoluteHumidity = 216.7f * ((hum_pct / 100.0f) * 6.112f * exp((17.62f * temp_c) / (243.12f + temp_c)) / (273.15f + temp_c)); // [g/m^3]
  const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
  return absoluteHumidityScaled;
}

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
      last_read_ms_ = millis();
      last_temp_f_ = TEMP_F(t.temperature);
      last_press_hpa_ = p.pressure;
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

// AHT20
FhAht20::FhAht20(void) {
}

uint8_t FhAht20::readAht20(void) {
  sensors_event_t t, h;
  if (getEvent(&h, &t)) {
    last_read_ms_ = millis();
    last_temp_f_ = TEMP_F(t.temperature);
    last_hum_pct_ = h.relative_humidity;
    return 0;
  } else {
    return 1;
  }
}
// End AHT20

#ifdef ADAFRUIT_SGP30_H
FhSgp30::FhSgp30(void) {
}

uint8_t FhSgp30::setupSgp30(void) {
  uint8_t i;
  uint8_t retval = 1;
  for (i = 1; i++; i <= 5 ) {
    if (begin()) {
      retval = 0;
      break;
    } else {
      Serial.println("Connect to SGP30 FAILED!");
      delay(100);
    }
  }
  return retval;
}

uint8_t FhSgp30::readSgp30(float temp_c = -1000, float hum_pct = -1) {
  uint8_t retval = 0;
  if ((hum_pct > 0) && (temp_c > -100)) {
    setHumidity(getAbsoluteHumidity(temp_c, hum_pct));
  }
  if (IAQmeasure()) {
    last_read_ms_ = millis();
    last_tvoc_ = TVOC;
    last_eco2_ = eCO2;
  } else {
    retval++;
  }
  if (IAQmeasureRaw()) {
    last_read_raw_ms_ = millis();
    last_raw_ethanol_ = rawEthanol;
    last_raw_h2_ = rawH2;
  } else {
    retval++;
  }
  return retval;
}
#endif /* ADAFRUIT_SGP30_H */

#ifdef ADAFRUIT_SHT4x_H
FhSht40::FhSht40(void) {
}

uint8_t FhSht40::setupSht40(void) {
  uint8_t i;
  uint8_t retval = 1;
  for (i = 1; i++; i <= 5 ) {
    if (begin()) {
      retval = 0;
      break;
    } else {
      Serial.println("Connect to SGP30 FAILED!");
      delay(100);
    }
  }
  return retval;
}

uint8_t FhSht40::readSht40(void) {
  sensors_event_t t, h;
  if (getEvent(&h, &t)) {
    last_read_ms_ = millis();
    last_temp_f_ = TEMP_F(t.temperature);
    last_hum_pct_ = h.relative_humidity;
    return 0;
  } else {
    return 1;
  }
}
#endif /* ADAFRUIT_SHT4x_H */