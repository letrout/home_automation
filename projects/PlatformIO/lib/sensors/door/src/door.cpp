#include <Arduino.h>

#include "door.h"
#include "owens_sensors.h"

#define MQTT_STR "%s,location=%s,room=%s,room_loc=%s, type=%s state=%d %lu%s"


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

uint16_t DoorSensor::set_mqtt_msg_len() {
  char msg[256];
  sprintf(msg, MQTT_STR,
    DOOR_OPEN_MEASUREMENT, location_, room_, room_loc_,  DOOR_OPEN_MEASUREMENT_TYPE, 1, 1703827402ul, "000000");
  return strlen(msg) + 2;
}

void DoorSensor::mqtt_msg_lp(char * mqtt_msg) 
{
  if (last_read_epoch_ms() == 0) {
    sprintf(mqtt_msg, "%s,location=%s,room=%s,room_loc=%s,type=%s state=%d",
    DOOR_OPEN_MEASUREMENT, location_, room_, room_loc_, DOOR_OPEN_MEASUREMENT_TYPE, last_read_state());
  } else {
    sprintf(mqtt_msg, "%s,location=%s,room=%s,room_loc=%s, type=%s state=%d %lu%s",
    DOOR_OPEN_MEASUREMENT, location_, room_, room_loc_,  DOOR_OPEN_MEASUREMENT_TYPE, last_read_state(), last_read_epoch_ms(), "000000");
  }
  Serial.println("mqtt_msg: ");
  Serial.println(mqtt_msg);
  return;
}

#ifdef PubSubClient_h
bool DoorSensor::mqtt_pub(PubSubClient &mqtt_client, const char * mqtt_topic) 
{
  char msg[mqtt_msg_len_];
  mqtt_msg_lp(msg);
  unsigned int len = strlen(msg);
  if (mqtt_client.publish(mqtt_topic, (uint8_t*)msg, len, false)) {
    last_publish_ms_ = millis();
#ifdef DOOR_OPEN_H
    Serial.print("MQTT publish ok: ");
    Serial.println(msg);
#endif
    return true;
  } else {
#ifdef DOOR_OPEN_H
    Serial.print("MQTT publish failed: ");
    Serial.println(msg);
#endif
    return false;
  }
}

#endif // PubSubClient_h
