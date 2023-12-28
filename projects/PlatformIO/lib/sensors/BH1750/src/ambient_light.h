#ifndef AMBIENT_LIGHT_H
#define AMBIENT_LIGHT_H 1

#include <string>
#include <BH1750.h>
#include "owens_sensors.h"

const char * const AMBIENT_LIGHT_MEASUREMENT = "environment";

class AmbientLight : public BH1750 {
  private:
    float last_ambient_lux_;
    unsigned long last_read_ms_;
    unsigned long last_read_epoch_ms_ = 0;

  public:
    /**
     * @brief Construct a new Ambient Light object
     * 
     * @param byte addr I2C address of the sensor
     */
    AmbientLight(byte addr = 0x23)
    : BH1750(addr) {
      last_ambient_lux_ = 0.0;
      last_read_ms_ = 0;
    };
    float last_ambient_lux() const { return last_ambient_lux_; }
    /**
     * @brief time of last read of the sensor, in millis()
     * 
     * @return unsigned long time in milliseconds
     */
    unsigned long last_read_ms() const { return last_read_ms_; }
    /**
     * @brief time of last read of sensor, in epoch milliseconds
     * 
     * @return unsigned long time in epoch milliseconds
     */
    unsigned long last_read_epoch_ms() const { return last_read_epoch_ms_; }
    /**
     * @brief read the ambient light sensor
     * 
     * @return int8_t E_SENSOR_SUCCESS or E_SENSOR_FAIL
     */
    int8_t read();
    /**
     * @brief MQTT message for the last read of the sensor
     * 
     * @param location location of the sensor
     * @param room room of the sensor
     * @param room_loc room location of the sensor
     * @return std::string MQTT message
     */
    std::string mqtt_msg_lp(
      const char * location,
      const char * room,
      const char * room_loc);
};

#endif // AMBIENT_LIGHT_H