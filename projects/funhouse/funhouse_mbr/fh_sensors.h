#ifndef FH_SENSORS_H
#define FH_SENSORS_H

#include <Adafruit_DPS310.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_SGP30.h>
#include <Adafruit_SHT4x.h>
#include <SensirionI2CScd4x.h>

#define TEMP_F(c) (c * 9 / 5) + 32
#define TEMP_C(f) (f - 32) * 5 / 9

/**
 * @brief Get the Absolute Humidity in mg/m^3
 * approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
 * 
 * @param temp_c Temperature in Celsius 
 * @param hum_pct Relative humidity %
 * @return uint32_t 
 */
uint32_t getAbsoluteHumidity(float temp_c, float hum_pct);

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
    /**
     * @brief Return pressure adjusted for altitude, inches Hg
     * 
     * @return float adjusted air pressure, in Hg
     */
    float inHgAdjusted(void);
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

#ifdef ADAFRUIT_SGP30_H
/**
 * @brief Class to extend Adafruit_SGP30
 * 
 */
class FhSgp30 : public Adafruit_SGP30 {
  private:
    uint16_t last_tvoc_;
    uint16_t last_eco2_;
    uint16_t last_raw_h2_;
    uint16_t last_raw_ethanol_;
    unsigned long last_read_ms_;
    unsigned long last_read_raw_ms_;

  public:
    FhSgp30();
    uint16_t last_tvoc() const { return last_tvoc_; }
    uint16_t last_eco2() const { return last_eco2_; }
    uint16_t last_raw_h2() const { return last_raw_h2_; }
    uint16_t last_raw_ethanol() const { return last_raw_ethanol_; }
    unsigned long last_read_ms() const { return last_read_ms_; }
    unsigned long last_read_raw_ms() const { return last_read_raw_ms_; }

    /**
     * @brief Initialize the sgp30 object
     * 
     * @return uint8_t 0 on success
     */
    uint8_t setupSgp30();

    /**
     * @brief Read the SGP30 sensor
     * If the temp and humidity are passed, a correction factor will be used
     * 
     * @param temp_c optional temperature in Celsius
     * @param hum_pct optional relative humidity
     * @return uint8_t 0 on success, 1 on failure
     */
    uint8_t readSgp30(float temp_c= -1000, float hum_pct = -1);
};
#endif /* ADAFRUIT_SGP30_H */

#ifdef ADAFRUIT_SHT4x_H
/**
 * @brief Class to extend Adafruit_SHT4x
 * 
 */
class FhSht40 : public Adafruit_SHT4x {
  private:
    float last_temp_f_;
    float last_hum_pct_;
    unsigned long last_read_ms_;

  public:
    FhSht40();
    float last_temp_f() const { return last_temp_f_; }
    float last_temp_c() const { return TEMP_C(last_temp_f_); }
    float last_hum_pct() const { return last_hum_pct_; }
    unsigned long last_read_ms() const { return last_read_ms_; }

    /**
     * @brief Initialize the sht40 object
     * 
     * @return uint8_t 0 on success
     */
    uint8_t setupSht40();

    uint8_t readSht40();
};
#endif /* ADAFRUIT_SHT4x_H */

#ifdef SENSIRIONI2CSCD4X_H
/**
 * @brief Class to extend SensirionI2CScd4x
 * 
 */
class FhScd40 : public SensirionI2CScd4x {
  private:
    uint16_t altitude_m_ = 0;
    float last_temp_f_;
    float last_hum_pct_;
    uint16_t last_co2_ppm_;
    unsigned long last_read_ms_;
    unsigned long last_update_ms_;

  public:
    FhScd40();
    float last_temp_f() const { return last_temp_f_; }
    float last_temp_c() const { return TEMP_C(last_temp_f_); }
    float last_hum_pct() const { return last_hum_pct_; }
    uint16_t last_co2_ppm() const { return last_co2_ppm_; }
    /**
     * @brief Last time (millis()) sensors was attemtped to be read (successfully or not)
     * 
     */
    unsigned long last_read_ms() const { return last_read_ms_; }
    /**
     * @brief Last time (millis()) sensor was *successfully* read
     * 
     */
    unsigned long last_update_ms() const { return last_update_ms_; }

    /**
     * @brief Initialize the SCD40 sensor
     * 
     * @param altitude_m optional - altitude in meters
     * @return uint16_t error code, 0 on success
     */
    uint16_t setupScd40(uint16_t altitude_m = 0);

    /**
     * @brief Read data from the SCD40
     * 
     * @param ambient_press_hpa optional - ambient pressure in hPa, for pressure compensation
     * @return uint16_t error - 0 on success, error code on failure
     */
    uint16_t readScd40(uint16_t ambient_press_hpa = 0);
};
#endif /* SENSIRIONI2CSCD4X_H */

class FhAmbientLight {
  private:
    uint16_t last_ambient_light_;
    unsigned long last_read_ms_;

  public:
    FhAmbientLight();
    uint16_t last_ambient_light() const { return last_ambient_light_; }
    unsigned long last_read_ms() const { return last_read_ms_; }
    /**
     * @brief read the ambient light sensor
     * 
     */
    void read();
};

#endif /* FH_SENSORS_H */