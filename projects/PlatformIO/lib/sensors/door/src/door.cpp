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

std::string DoorSensor::mqtt_msg_lp(
  const char * location,
  const char * room,
  const char * room_loc) 
{
  char mqtt_msg [128];
  if (last_read_epoch_ms() == 0) {
    sprintf(mqtt_msg, "%s,sensor=light,location=%s,room=%s,room_loc=%s,type=%s state=%d",
    DOOR_OPEN_MEASUREMENT, location, room, room_loc, DOOR_OPEN_MEASUREMENT_TYPE, last_read_state());
  } else {
    sprintf(mqtt_msg, "%s,sensor=light,location=%s,room=%s,room_loc=%s, type=%s state=%d %lu%s",
    DOOR_OPEN_MEASUREMENT, location, room, room_loc,  DOOR_OPEN_MEASUREMENT_TYPE, last_read_state(), last_read_epoch_ms(), "000000");
  }
  return std::string(mqtt_msg);
}
