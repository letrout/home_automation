#include "Luth_SHT30.h"
#include "owens_sensors.h"


int8_t LuthSht30::read(void) {
  if (millis() - last_read_ms() < SHT30_MIN_READ_MS) {
    // Too soon since last read
    return E_SENSOR_NOOP;
  }
  uint16_t status = readStatus();
  Serial.print(status, HEX);
  Serial.println();
  // if ((status == 0) & SHT31::read(true)) {
  if (SHT31::read(true)) {
    last_temp_f_ = getFahrenheit();
    last_hum_rel_ = getHumidity();
    last_read_ms_ = millis();
    // TODO: set last_read_epoch_ms_ to the current time
    return E_SENSOR_SUCCESS;
  }
  return status;
}

void LuthSht30::mqtt_msg_lp(char * mqtt_msg) 
{
  if (last_read_epoch_ms() == 0) {
    sprintf(mqtt_msg, "%s,sensor=SHT30,location=%s,room=%s,room_loc=%s temp_f=%f humidity=%f",
    SHT30_MEASUREMENT, location_, room_, room_loc_, last_temp_f(), last_hum_rel());
  } else {
    sprintf(mqtt_msg, SHT30_MQTT_STR,
    SHT30_MEASUREMENT, location_, room_, room_loc_, last_temp_f(), last_hum_rel(), last_read_epoch_ms(), "000000");
  }
  return;
}

#ifdef PubSubClient_h
bool LuthSht30::mqtt_pub(PubSubClient &mqtt_client, const char * mqtt_topic) 
{
  char msg[mqtt_msg_len_];
  mqtt_msg_lp(msg);
  unsigned int len = strlen(msg);
  if (mqtt_client.publish(mqtt_topic, (uint8_t*)msg, len, false)) {
    last_publish_ms_ = millis();
    return true;
  } else {
    return false;
  }
}
#endif // PubSubClient_h

// End ambient light sensor
