#include "Luth_SGP30.h"
#include "owens_sensors.h"


int8_t LuthSgp30::read(bool all) {
  if (millis() - last_read_ms() < SGP30_MIN_READ_MS) {
    // Too soon since last read
    return E_SENSOR_NOT_READY;
  }
  if (measure(all)) {
    last_read_ms_ = millis();
    //if (getTVOC() == 60000) {
      // Sometime sensor gets stuck at max value
    //  GenericReset();
    //  return E_SENSOR_FAIL;
    //}
    last_tvoc_ = getTVOC();
    last_eco2_ = getCO2();
    if (all) {
      last_h2_ = getH2_raw();
      last_ethanol_ = getEthanol_raw();
    }
    // TODO: set last_read_epoch_ms_ to the current time
    return E_SENSOR_SUCCESS;
  }
  return E_SENSOR_FAIL;
}

void LuthSgp30::mqtt_msg_lp(char * mqtt_msg) 
{
  if (last_read_epoch_ms() == 0) {
    sprintf(mqtt_msg, "%s,sensor=SGP30,location=%s,room=%s,room_loc=%s tvoc=%u eco2=%u",
    SGP30_MEASUREMENT, location_, room_, room_loc_, last_tvoc(), last_eco2());
  } else {
    sprintf(mqtt_msg, SGP30_MQTT_STR,
    SGP30_MEASUREMENT, location_, room_, room_loc_, last_tvoc(), last_eco2(), last_read_epoch_ms(), "000000");
  }
  return;
}

void LuthSgp30::mqtt_msg_raw_lp(char * mqtt_msg) 
{
  if (last_read_epoch_ms() == 0) {
    sprintf(mqtt_msg, "%s,sensor=SGP30,location=%s,room=%s,room_loc=%s h2=%u ethanol=%u",
    SGP30_MEASUREMENT, location_, room_, room_loc_, last_h2(), last_ethanol());
  } else {
    sprintf(mqtt_msg, SGP30_MQTT_RAW_STR,
    SGP30_MEASUREMENT, location_, room_, room_loc_, last_h2(), last_ethanol(), last_read_epoch_ms(), "000000");
  }
  return;
}

uint8_t LuthSgp30::unable_to_pub() {
  if (last_read_ms_ < last_publish_ms_) {
    // Don't publish old data
    return E_SENSOR_NOT_READY;
  } else if ((last_eco2_ == 0) | (last_tvoc_ >= 60000)) {
    // Don't publish erroneous data
    return E_SENSOR_FAIL;
  } else {
    // Ok to publish data
    return E_SENSOR_SUCCESS;
  }
}

#ifdef PubSubClient_h
bool LuthSgp30::mqtt_pub(PubSubClient &mqtt_client, const char * mqtt_topic, bool all) 
{
  if (unable_to_pub() != E_SENSOR_SUCCESS) {
    return false;
  }
  bool retval = false;
  char msg[mqtt_msg_len_];
  mqtt_msg_lp(msg);
  unsigned int len = strlen(msg);
  if (mqtt_client.publish(mqtt_topic, (uint8_t*)msg, len, false)) {
    last_publish_ms_ = millis();
    retval = true;
  }
  if (all) {
    char msg_raw[mqtt_msg_raw_len_];
    mqtt_msg_raw_lp(msg_raw);
    len = strlen(msg_raw);
    if (mqtt_client.publish(mqtt_topic, (uint8_t*)msg_raw, len, false)) {
      if (!retval) {
        last_publish_ms_ = millis();
      }
      retval = true;
    }
  }
  return retval;
}
#endif // PubSubClient_h

// End SGP30 sensor
