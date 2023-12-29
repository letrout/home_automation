#ifndef AMBIENT_LIGHT_H
#define AMBIENT_LIGHT_H 1

#include <PubSubClient.h>
#include <BH1750.h>
#include "owens_sensors.h"

#define AMBIENT_LIGHT_MIN_READ_MS 1000 // minimum time between reads, in milliseconds
const char * const AMBIENT_LIGHT_MEASUREMENT = "environment";

class AmbientLight : public BH1750 {
  private:
    const char * location_;
    const char * room_;
    const char * room_loc_;
    float last_ambient_lux_ = -1.0;
    unsigned long last_read_ms_ = 0;
    unsigned long last_read_epoch_ms_ = 0;
    unsigned int last_publish_ms_ = 0;

  public:
    /**
     * @brief Construct a new Ambient Light object
     * 
     * @param byte addr I2C address of the sensor
     */
    AmbientLight(const char * location, const char * room, const char * room_loc, byte addr = 0x23)
    : BH1750(addr) {
      location_ = location;
      room_ = room;
      room_loc_ = room_loc;
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
     * @return std::string MQTT message
     */
    char * mqtt_msg_lp();
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

#endif // AMBIENT_LIGHT_H