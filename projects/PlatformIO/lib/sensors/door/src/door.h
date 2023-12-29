#ifndef DOOR_OPEN_H
#define DOOR_OPEN_H 1

#include <PubSubClient.h>
#include "owens_sensors.h"

const char * const DOOR_OPEN_MEASUREMENT = "sensor";
const char * const DOOR_OPEN_MEASUREMENT_TYPE = "door";

class DoorSensor {
  private:
    uint8_t door_pin_;
    const char * location_;
    const char * room_;
    const char * room_loc_;
    uint16_t mqtt_msg_len_ = 0;
    int8_t last_read_state_ = -1; // 0 - closed, 1 - open
    unsigned int last_read_ms_ = 0;
    unsigned long last_read_epoch_ms_ = 0;
    unsigned int last_publish_ms_ = 0;
    uint16_t set_mqtt_msg_len();

  public:
    DoorSensor(const uint8_t door_pin, const char * location, const char * room, const char * room_loc) {
        door_pin_ = door_pin;
        location_ = location;
        room_ = room;
        room_loc_ = room_loc;
        mqtt_msg_len_ = set_mqtt_msg_len();
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
     * @brief time of last publish of the sensor, in millis()
     * 
     * @return unsigned long time in milliseconds
     */
    unsigned long last_publish_ms() const { return last_publish_ms_; }
    /**
     * @brief MQTT message for the last read of the sensor
     * 
     * @param mqtt_msg char array to hold the MQTT message
     * @return
     */
    void mqtt_msg_lp(char * mqtt_msg);
    /**
     * @brief Length needed for MQTT message string
     * 
     * @return uint16_t MQTT message length
     */
    uint16_t mqtt_msg_len() const { return mqtt_msg_len_; }
#ifdef PubSubClient_h
    /**
     * @brief Publish MQTT message for the last read of the sensor
     * 
     * @param mqtt_client PubSubClient object
     * @return bool true if message was successfully published
     */
    bool mqtt_pub(PubSubClient & mqtt_client, const char * mqtt_topic);
#endif // PUBSUBCLIENT_H

};

#endif // DOOR_OPEN_H
