#ifndef LUTH_SGP30_H
#define LUTH_SGP30_H 1

#include <PubSubClient.h>
#include <SGP30.h>
#include "owens_sensors.h"

#define SGP30_MIN_READ_MS 10000 // minimum time between reads, in milliseconds
#define SGP30_MQTT_STR "%s,sensor=SGP30,location=%s,room=%s,room_loc=%s tvoc=%u eco2=%u %lu%s"
#define SGP30_MQTT_RAW_STR "%s,sensor=SGP30,location=%s,room=%s,room_loc=%s h2=%u ethanol=%u %lu%s"
const char * const SGP30_MEASUREMENT = "environment";


class LuthSgp30 : public SGP30 {
  private:
    const char * location_;
    const char * room_;
    const char * room_loc_;
    uint16_t mqtt_msg_len_ = 0;
    uint16_t mqtt_msg_raw_len_ = 0;
    uint16_t last_tvoc_ = 0;
    uint16_t last_eco2_ = 0;
    uint16_t last_h2_ = 0;
    uint16_t last_ethanol_ = 0;
    unsigned long last_read_ms_ = 0;
    unsigned long last_read_epoch_ms_ = 0;
    unsigned int last_publish_ms_ = 0;

  public:
    /**
     * @brief Construct a new Ambient Light object
     * 
     * @param byte addr I2C address of the sensor
     */
    LuthSgp30(const char * location, const char * room, const char * room_loc, byte addr = 0x58)
    : SGP30() {
      location_ = location;
      room_ = room;
      room_loc_ = room_loc;
      char msg[256];
      char msg_raw[256];
      sprintf(msg, SGP30_MQTT_STR, SGP30_MEASUREMENT, location_, room_,
      room_loc_, 60000, 60000, 1703827402ul, "000000");
      mqtt_msg_len_= strlen(msg) + 2;
      sprintf(msg_raw, SGP30_MQTT_RAW_STR, SGP30_MEASUREMENT, location_, room_,
      room_loc_, 64000, 64000, 1703827402ul, "000000");
      mqtt_msg_raw_len_= strlen(msg_raw) + 2;
    };
    uint16_t last_tvoc() const { return last_tvoc_; }
    uint16_t last_eco2() const { return last_eco2_; }
    uint16_t last_h2() const { return last_h2_; }
    uint16_t last_ethanol() const { return last_ethanol_; }
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
     * @brief time of last publish of the sensor, in millis()
     * 
     * @return unsigned long time in milliseconds
     */
    unsigned long last_publish_ms() const { return last_publish_ms_; }
    /**
     * @brief read the ambient light sensor
     * 
     * @return int8_t E_SENSOR_SUCCESS or E_SENSOR_FAIL
     */
    int8_t read(bool all = true);
    /**
     * @brief MQTT message for the last read of the sensor
     * 
     * @param mqtt_msg char array to store the MQTT message
     * @return 
     */
    void mqtt_msg_lp(char * mqtt_msg);
    /**
     * @brief MQTT message for the last raw read of the sensor
     * 
     * @param mqtt_msg char array to store the MQTT message
     * @return 
     */
    void mqtt_msg_raw_lp(char * mqtt_msg);
    /**
     * @brief Check if we have data suitable to publish
     * 
     * @return E_SENSOR_SUCCES if ready to publish, something else if not
    */
    uint8_t unable_to_pub();
#ifdef PubSubClient_h
    /**
     * @brief Publish MQTT message for the last read of the sensor
     * 
     * @param mqtt_client PubSubClient object
     * @return bool true if message was successfully published
     */
    bool mqtt_pub(PubSubClient & mqtt_client, const char * mqtt_topic, bool all = true);
#endif // PUBSUBCLIENT_H
};

#endif // LUTH_SGP30_H
