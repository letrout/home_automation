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

uint8_t AmbientLight::unable_to_pub() {
  if (last_read_ms_ < last_publish_ms_) {
    // Don't publish old data
    return E_SENSOR_NOT_READY;
  } else if (last_ambient_lux_ < 0) {
    // Don't publish erroneous data
    return E_SENSOR_FAIL;
  } else {
    // Ok to publish data
    return E_SENSOR_SUCCESS;
  }
}


#ifdef PubSubClient_h
bool AmbientLight::mqtt_pub(PubSubClient &mqtt_client, const char * mqtt_topic) 
{
  if (unable_to_pub() != E_SENSOR_SUCCESS) {
    return false;
  }
  char msg[mqtt_msg_len_];
  mqtt_msg_lp(msg);
  unsigned int len = strlen(msg);
  if (mqtt_client.publish(mqtt_topic, (uint8_t*)msg, len, false)) {
    last_publish_ms_ = millis();
    // Serial.print("MQTT publish ok: ");
    // Serial.println(msg);
    return true;
  } else {
    // Serial.print("MQTT publish failed: ");
    // Serial.println(msg);
    return false;
  }
}
#endif // PubSubClient_h

// End ambient light sensor
