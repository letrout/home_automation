#ifndef PIR_MOTION_H
#define PIR_MOTION_H 1

#include <PubSubClient.h>
#include "owens_sensors.h"

#define PIR_MQTT_STR "%s,location=%s,room=%s,room_loc=%s, type=%s state=%d %lu%s"

const char * const PIR_MEASUREMENT = "events";
const char * const PIR_MEASUREMENT_TYPE = "motion";

class PirSensor {
  private:
    uint8_t pir_pin_;
    const char * location_;
    const char * room_;
    const char * room_loc_;
    uint16_t mqtt_msg_len_ = 0;
    int8_t last_read_state_ = -1; // 0 - no motion, 1 - motion
    unsigned long last_read_ms_ = 0;
    unsigned long last_read_epoch_ms_ = 0;
    unsigned long last_publish_ms_ = 0;
    unsigned long last_motion_ms_ = 0;

  public:
    PirSensor(const uint8_t pir_pin, const char * location, const char * room, const char * room_loc) {
        pir_pin_ = pir_pin;
        location_ = location;
        room_ = room;
        room_loc_ = room_loc;
        char msg[256];
        sprintf(msg, PIR_MQTT_STR, PIR_MEASUREMENT, location_, room_, room_loc_,
        PIR_MEASUREMENT_TYPE, 1, 1703827402ul, "000000");
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
     * @brief time of last motion detection, in millis()
     * 
     * @return unsigned long time in milliseconds
     */
    unsigned long last_motion_ms() const { return last_motion_ms_; }
    /**
     * @brief time of last publish of the sensor, in millis()
     * 
     * @return unsigned long time in milliseconds
     */
    unsigned long last_publish_ms() const { return last_publish_ms_; }
    /**
     * @brief Check if we have data suitable to publish
     * 
     * @return E_SENSOR_SUCCES if ready to publish, something else if not
    */
    uint8_t unable_to_pub();
    /**
     * @brief MQTT message for the last read of the sensor
     * 
     * @param mqtt_msg char array to store the MQTT message
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

#endif // PIR_MOTION_H