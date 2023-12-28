#ifndef DOOR_OPEN_H
#define DOOR_OPEN_H 1

#include <string>
#include "owens_sensors.h"

const char * const DOOR_OPEN_MEASUREMENT = "sensor";
const char * const DOOR_OPEN_MEASUREMENT_TYPE = "door";

class DoorSensor {
  private:
    uint8_t door_pin_;
    int8_t last_read_state_ = -1; // 0 - closed, 1 - open
    unsigned int last_read_ms_ = 0;
    unsigned long last_read_epoch_ms_ = 0;

  public:
    DoorSensor(uint8_t door_pin) {
        door_pin_ = door_pin;
    }
    /**
     * @brief initialize the sensor
     * 
     * @return int8_t E_SENSOR_SUCCESS or E_SENSOR_FAIL
     */
    int8_t begin();
    /**
     * @brief read the sensor
     * 
     * @return int8_t E_SENSOR_SUCCESS or E_SENSOR_FAIL
     */
    int8_t read();
    /**
     * @brief get the last state of the sensor
     * 
     * @return 0: open, 1: closed, -1: unknown
     */
    int8_t last_read_state() const { return last_read_state_; }
    /**
     * @brief time of the last read of the sensor, in millis()
     * 
     * @return unsigned long time in milliseconds
     */
    unsigned long last_read_ms() const { return last_read_ms_; }
    /**
     * @brief time of read the sensor, in epoch milliseconds
     * 
     * @return unsigned long time in epoch milliseconds
     */
    unsigned long last_read_epoch_ms() const { return last_read_epoch_ms_; }
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

#endif // DOOR_OPEN_H
