/**
 * @file fh_sensors.cpp
 * @author Joel Luth (joel.luth@gmail.com)
 * @brief Classes to manage sensors used on Adafruit Funhouse
 * @version 1.0
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "fh_sensors.h"

#define HPA_ALT_ADJUST 34 // add this to hPa air pressue to adjust for (our specific) altitude

// sensors objects
FhAmbientLight ambientLight;
FhDps310 dps;
FhAht20 aht;
#ifdef ADAFRUIT_SGP30_H
FhSgp30 sgp30;
#endif
#ifdef ADAFRUIT_SHT4x_H
FhSht40 sht4x = FhSht40();
#endif
#ifdef SENSIRIONI2CSCD4X_H
FhScd40 scd4x;
#define SCD4X_OFFSET_C 3.4  // Stock is 4C(?), testing shows 3.4 better matches my SHT40
const unsigned long scd4x_min_read_ms = 5000;  // minimum interval between SCD4x reads, in ms
#endif
#ifdef ADAFRUIT_PM25AQI_H
FhPm25Aqi pm25Aqi = FhPm25Aqi();
const unsigned long pm25aqi_min_read_ms = 10000;  // minimum interval between PMSA003I reads, in ms
#endif

uint32_t getAbsoluteHumidity(float temp_c, float hum_pct) {
  // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
  const float absoluteHumidity = 216.7f * ((hum_pct / 100.0f) * 6.112f * exp((17.62f * temp_c) / (243.12f + temp_c)) / (273.15f + temp_c)); // [g/m^3]
  const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
  return absoluteHumidityScaled;
}

// Ambient light sensor
FhAmbientLight::FhAmbientLight(void) {
}

void FhAmbientLight::read(void) {
  last_ambient_light_ = analogRead(A3);
  last_read_ms_ = millis();
  return;
}
// End ambient light sensor

// DPS310
FhDps310::FhDps310(void) {
}

uint8_t FhDps310::setupDps310(uint8_t retries) {
  uint8_t retval = E_SENSOR_FAIL;
  for (uint8_t i = 0; i <= retries; i++ ) {
    if (begin_I2C()) {
      present_ = true;
      retval = E_SENSOR_SUCCESS;
      break;
    } else {
      Serial.println("Connect to DPS310 FAILED!");
      delay(100);
    }
  }
  if (retval == E_SENSOR_SUCCESS) {
    configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
    configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);
  }
  return retval;
}

uint8_t FhDps310::readDps310(void) {
  if (!present_) {
    return E_SENSOR_NOT_PRESENT;
  }
  sensors_event_t t, p;
  if (pressureAvailable() && temperatureAvailable()) {
    if (getEvents(&t, &p)) {
      last_read_ms_ = millis();
      last_temp_f_ = TEMP_F(t.temperature);
      last_press_hpa_ = p.pressure;
      return E_SENSOR_SUCCESS;
    } else {
      Serial.println("DPS310 read failed!");
      return E_SENSOR_FAIL;
    }
  } else {
    Serial.println("DPS310 temp or pressure not available");
    return E_SENSOR_FAIL + 1;
  }
}

float FhDps310::inHgAdjusted(void) {
  return (float(last_press_hpa()) + HPA_ALT_ADJUST) * 0.02953;
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
    return E_SENSOR_SUCCESS;
  } else {
    return E_SENSOR_FAIL;
  }
}
// End AHT20

#ifdef ADAFRUIT_SGP30_H
FhSgp30::FhSgp30(void) {
}

uint8_t FhSgp30::setupSgp30(uint8_t retries) {
  uint8_t retval = E_SENSOR_FAIL;
  for (uint8_t i = 0; i <= retries; i++ ) {
    if (begin()) {
      retval = E_SENSOR_SUCCESS;
      present_ = true;
      break;
    } else {
      Serial.println("Connect to SGP30 FAILED!");
      delay(100);
    }
  }
  return retval;
}

uint8_t FhSgp30::readSgp30(float temp_c, float hum_pct) {
  if (!present_) {
    return E_SENSOR_NOT_PRESENT;
  }
  uint8_t retval = E_SENSOR_SUCCESS;
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

uint8_t FhSht40::setupSht40(uint8_t retries) {
  uint8_t retval = E_SENSOR_FAIL;
  for (uint8_t i = 0; i <= retries; i++ ) {
    if (begin()) {
      retval = E_SENSOR_SUCCESS;
      present_ = true;
      break;
    } else {
      Serial.println("Connect to SHT4x FAILED!");
      delay(100);
    }
  }
  return retval;
}

uint8_t FhSht40::readSht40(void) {
  if (!present_) {
    return E_SENSOR_NOT_PRESENT;
  }
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

#ifdef SENSIRIONI2CSCD4X_H
FhScd40::FhScd40(void) {
}

uint16_t FhScd40::setupScd40(uint8_t retries, uint16_t altitude_m) {
  uint16_t error = E_SENSOR_SUCCESS;
  char errorMessage[256];
  Serial.println("Setting up SCD-40...");
  begin(Wire);
  for (uint8_t i = 0; i <= retries; i++ ) {
    Serial.printf("SCD4x setup retry %d\n", i);
    // stop potentially previously started measurement
    error = stopPeriodicMeasurement();
    if (error) {
      Serial.print("Error trying to execute SCD40 stopPeriodicMeasurement(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
      continue;
    }
    // Set altitude
    if (altitude_m > 0) {
      altitude_m_ = altitude_m;
      error = setSensorAltitude(altitude_m);
      if (error) {
        Serial.print("Error trying to execute SCD40 setSensorAltitude(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        continue;
      }
      if (!error) {
        Serial.println("Inital SCD-4x setup complete");
        break;
      }
    }
    // Set the temperature offset
    if (setTemperatureOffset(SCD4X_OFFSET_C)) {
      Serial.printf("Error setting temp offset to %0.2f C\n", SCD4X_OFFSET_C);
    }
    // Start Measurement
    error = startPeriodicMeasurement();
    if (error) {
      Serial.print("Error trying to execute SCD40 startPeriodicMeasurement(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
      continue;
    }
  }
  if (error) {
    present_ = false;
  } else {
    present_ = true;
  }
  return error;
}

uint16_t FhScd40::reInitialize(void) {
  uint16_t error = E_SENSOR_SUCCESS;
  char errorMessage[256];
  // stop potentially previously started measurement
  Serial.println("Attempting to stop SCD40 periodic measurement");
  error = stopPeriodicMeasurement();
  if (error) {
    Serial.print("Error trying to execute SCD40 stopPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else {
    // Start Measurement
    delay(1000);
    Serial.println("Attempting to start SCD40 periodic measurement");
    error = startPeriodicMeasurement();
    if (error) {
      Serial.print("Error trying to execute SCD40 startPeriodicMeasurement(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
    }
  }
  if (error) {
    present_ = false;
  } else {
    present_ = true;
  }
  return error;
}

uint16_t FhScd40::readScd40(uint16_t ambient_press_hpa) {
  if (!present_) {
    return E_SENSOR_NOT_PRESENT;
  }
  float t, h;
  uint16_t c = 0;
  uint16_t error;
  // Protect against reading the SCD4x too quickly
  if ((millis() - last_read_ms_) < scd4x_min_read_ms) {
    Serial.printf("Error - trying to re-read SCD4x before %lu ms have elapsed\n", scd4x_min_read_ms);
    return E_SENSOR_FAIL;
  }
  if (ambient_press_hpa > 0) {
    if (setAmbientPressure(ambient_press_hpa)) {
      Serial.println("Error setting SCD40 ambient pressure compensation");
    } else if (altitude_m_ > 0) {
      // if we fail to set pressure comp, try to re-set via altitude
      // Note - this cannot be run while periodic measurements are active, so don't try
      // setSensorAltitude(altitude_m_);
    }
  }
  /* *** not sure how this should work; return val? dataReady?
  uint16_t dataReady = 0;
  getDataReadyStatus(dataReady);
  if (dataReady) {
    Serial.printf("SCD41x getDataReadyStatus() error: %d\n", dataReady);
    // TODO: if read error, try a stop/start to reset the sensor?
    return dataReady;
  }
  */
  error = readMeasurement(c, t, h);
  last_read_ms_ = millis();
  if (! error && (c != 0)) {
    last_update_ms_ = last_read_ms_;
    last_co2_ppm_ = c;
    last_temp_f_ = TEMP_F(t);
    last_hum_pct_ = h;
  } else {
    // typically fails with 526 from receiveFrame(), which is I think ReadError | NotEnoughDataError
    // In my experience, when this fails it stays failed (at least until a power cycle)
    // So, attempt to kick-start the sensor
    reInitialize();
    // TODO: check this return? retry on failure? re-read on success?

  }
  return error;
}
#endif /* SENSIRIONI2CSCD4X_H */

#ifdef ADAFRUIT_PM25AQI_H
FhPm25Aqi::FhPm25Aqi(void) {
}

bool FhPm25Aqi::begin_I2C() {
  present_ = Adafruit_PM25AQI::begin_I2C();
  return present();
}

bool FhPm25Aqi::read() {
  // Protect against reading the PMSA003I too quickly
  if ((millis() - last_read_ms_) < pm25aqi_min_read_ms) {
    Serial.printf("Error - trying to re-read PMSA003I before %lu ms have elapsed\n", pm25aqi_min_read_ms);
    return false;
  }
  if (Adafruit_PM25AQI::read(&last_data_)) {
    last_read_ms_ = millis();
    return true;
  } else {
    return false;
  }
}
#endif