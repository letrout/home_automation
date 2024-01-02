#ifndef LUTH_SHT30_H
#define LUTH_SHT30_H 1

#include <PubSubClient.h>
#include <SHT31.h>
#include "owens_sensors.h"

#define SHT30_MIN_READ_MS 10000 // minimum time between reads, in milliseconds
#define SHT30_MQTT_STR "%s,sensor=SHT30,location=%s,room=%s,room_loc=%s temp_f=%f humidity=%f %lu%s"
const char * const SHT30_MEASUREMENT = "environment";


class LuthSht30 : public SHT31 {
  private:
    const char * location_;
    const char * room_;
    const char * room_loc_;
    uint16_t mqtt_msg_len_ = 0;
    float last_temp_f_ = -1.0;
    float last_hum_rel_ = -1.0;
    unsigned long last_read_ms_ = 0;
    unsigned long last_read_epoch_ms_ = 0;
    unsigned int last_publish_ms_ = 0;

  public:
    /**
     * @brief Construct a new Ambient Light object
     * 
     * @param byte addr I2C address of the sensor
     */
    LuthSht30(const char * location, const char * room, const char * room_loc, byte addr = 0x45)
    : SHT31(addr) {
      location_ = location;
      room_ = room;
      room_loc_ = room_loc;
      char msg[256];
      sprintf(msg, SHT30_MQTT_STR, SHT30_MEASUREMENT, location_, room_,
      room_loc_, 123.123456, 123.123456, 1703827402ul, "000000");
      mqtt_msg_len_= strlen(msg) + 2;
    };
    float last_temp_f() const { return last_temp_f_; }
    float last_hum_rel() const { return last_hum_rel_; }
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
    int8_t read();
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

#endif // LUTH_SHT30_H