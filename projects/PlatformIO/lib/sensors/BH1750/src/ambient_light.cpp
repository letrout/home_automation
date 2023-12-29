#include "ambient_light.h"
#include "owens_sensors.h"


int8_t AmbientLight::read(void) {
  if (millis() - last_read_ms() < AMBIENT_LIGHT_MIN_READ_MS) {
    // Too soon since last read
    return E_SENSOR_NOOP;
  }
  if (measurementReady()) {
    last_ambient_lux_ = readLightLevel();
    last_read_ms_ = millis();
    // TODO: set last_read_epoch_ms_ to the current time
    return E_SENSOR_SUCCESS;
  }
  return E_SENSOR_FAIL;
}

void AmbientLight::mqtt_msg_lp(char * mqtt_msg) 
{
  if (last_read_epoch_ms() == 0) {
    sprintf(mqtt_msg, "%s,sensor=BH1750,location=%s,room=%s,room_loc=%s lux=%lf",
    AMBIENT_LIGHT_MEASUREMENT, location_, room_, room_loc_, last_ambient_lux());
  } else {
    sprintf(mqtt_msg, "%s,sensor=BH1750,location=%s,room=%s,room_loc=%s lux=%f %lu%s",
    AMBIENT_LIGHT_MEASUREMENT, location_, room_, room_loc_, last_ambient_lux(), last_read_epoch_ms(), "000000");
  }
  return;
}

#ifdef PubSubClient_h
bool AmbientLight::mqtt_pub(PubSubClient &mqtt_client, const char * mqtt_topic) 
{
  char msg[mqtt_msg_len_];
  mqtt_msg_lp(msg);
  unsigned int len = strlen(msg);
  if (mqtt_client.publish(mqtt_topic, (uint8_t*)msg, len, false)) {
    last_publish_ms_ = millis();
#ifdef DEBUG
    Serial.print("MQTT publish ok: ");
    Serial.println(msg);
#endif
    return true;
  } else {
#ifdef DEBUG
    Serial.print("MQTT publish failed: ");
    Serial.println(msg);
#endif
    return false;
  }
}
#endif // PubSubClient_h

// End ambient light sensor
