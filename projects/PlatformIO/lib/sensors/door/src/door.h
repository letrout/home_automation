#ifndef DOOR_OPEN_H
#define DOOR_OPEN_H 1

#include <PubSubClient.h>
#include "owens_sensors.h"

#define DOOR_MQTT_STR "%s,location=%s,room=%s,room_loc=%s, type=%s state=%d %lu%s"

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

  public:
    DoorSensor(const uint8_t door_pin, const char * location, const char * room, const char * room_loc) {
        door_pin_ = door_pin;
        location_ = location;
        room_ = room;
        room_loc_ = room_loc;
        char msg[256];
        sprintf(msg, DOOR_MQTT_STR, DOOR_OPEN_MEASUREMENT, location_, room_, room_loc_,
        DOOR_OPEN_MEASUREMENT_TYPE, 1, 1703827402ul, "000000");
        mqtt_msg_len_ = strlen(msg) + 2;
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
