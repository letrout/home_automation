#include "ambient_light.h"
#include "owens_sensors.h"


// Ambient light sensor
//AmbientLight::AmbientLight(byte addr = 0x23) {
//}

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

std::string AmbientLight::mqtt_msg_lp() 
{
  char mqtt_msg [128];
  if (last_read_epoch_ms() == 0) {
    sprintf(mqtt_msg, "%s,sensor=BH1750,location=%s,room=%s,room_loc=%s lux=%lf",
    AMBIENT_LIGHT_MEASUREMENT, location_, room_, room_loc_, last_ambient_lux());
  } else {
    sprintf(mqtt_msg, "%s,sensor=BH1750,location=%s,room=%s,room_loc=%s lux=%f %lu%s",
    AMBIENT_LIGHT_MEASUREMENT, location_, room_, room_loc_, last_ambient_lux(), last_read_epoch_ms(), "000000");
  }
  return std::string(mqtt_msg);
}

#ifdef PubSubClient_h
bool AmbientLight::mqtt_pub(PubSubClient &mqtt_client, const char * mqtt_topic) 
{
  unsigned int len = strlen(mqtt_msg_lp().c_str());
  if (mqtt_client.publish(mqtt_topic, (uint8_t*)mqtt_msg_lp().c_str(), len, false)) {
    last_publish_ms_ = millis();
#ifdef DEBUG
    Serial.print("MQTT publish ok: ");
    Serial.println(mqtt_msg_lp().c_str());
#endif
    return true;
  } else {
#ifdef DEBUG
    Serial.print("MQTT publish failed: ");
    Serial.println(mqtt_msg_lp().c_str());
#endif
    return false;
  }
}
#endif // PubSubClient_h

// End ambient light sensor
