#include <Arduino.h>

#include "door.h"
#include "owens_sensors.h"


int8_t DoorSensor::begin(void) {
  pinMode(door_pin_, INPUT_PULLUP);
  return E_SENSOR_SUCCESS;
}

int8_t DoorSensor::read(void) {
  if (digitalRead(door_pin_) == HIGH) {
    last_read_ms_ = millis();
    last_read_state_ = 1;
    Serial.print("Door open: ");
  } else {
    last_read_ms_ = millis();
    last_read_state_ = 0;
    Serial.print("Door closed: ");
  }
  return E_SENSOR_SUCCESS;
}

char * DoorSensor::mqtt_msg_lp() 
{
  char mqtt_msg [128];
  if (last_read_epoch_ms() == 0) {
    sprintf(mqtt_msg, "%s,location=%s,room=%s,room_loc=%s,type=%s state=%d",
    DOOR_OPEN_MEASUREMENT, location_, room_, room_loc_, DOOR_OPEN_MEASUREMENT_TYPE, last_read_state());
  } else {
    sprintf(mqtt_msg, "%s,location=%s,room=%s,room_loc=%s, type=%s state=%d %lu%s",
    DOOR_OPEN_MEASUREMENT, location_, room_, room_loc_,  DOOR_OPEN_MEASUREMENT_TYPE, last_read_state(), last_read_epoch_ms(), "000000");
  }
  return mqtt_msg;
}

#ifdef PubSubClient_h
bool DoorSensor::mqtt_pub(PubSubClient &mqtt_client, const char * mqtt_topic) 
{
  unsigned int len = strlen(mqtt_msg_lp());
  if (mqtt_client.publish(mqtt_topic, (uint8_t*)mqtt_msg_lp(), len, false)) {
    last_publish_ms_ = millis();
#ifdef DOOR_OPEN_H
    Serial.print("MQTT publish ok: ");
    Serial.println(mqtt_msg_lp());
#endif
    return true;
  } else {
#ifdef DEBUG
    Serial.print("MQTT publish failed: ");
    Serial.println(mqtt_msg_lp());
#endif
    return false;
  }
}

#endif // PubSubClient_h
