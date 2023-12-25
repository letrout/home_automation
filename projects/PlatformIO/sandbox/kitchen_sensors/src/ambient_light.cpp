#include "ambient_light.h"
#include "owens_sensors.h"


// Ambient light sensor
//AmbientLight::AmbientLight(byte addr = 0x23) {
//}

int8_t AmbientLight::read(void) {
  if (measurementReady()) {
    last_ambient_lux_ = readLightLevel();
    last_read_ms_ = millis();
    // TODO: set last_read_epoch_ms_ to the current time
    return E_SENSOR_SUCCESS;
  }
  return E_SENSOR_FAIL;
}

std::string AmbientLight::mqtt_msg_lp(
  const char * location,
  const char * room,
  const char * room_loc) 
{
  char mqtt_msg [128];
  if (last_read_epoch_ms() == 0) {
    sprintf(mqtt_msg, "%s,sensor=light,location=%s,room=%s,room_loc=%s,type=light lux=%lf",
    AMBIENT_LIGHT_MEASUREMENT, location, room, room_loc, last_ambient_lux());
  } else {
    sprintf(mqtt_msg, "%s,sensor=light,location=%s,room=%s,room_loc=%s lux=%f %lu%s",
    AMBIENT_LIGHT_MEASUREMENT, location, room, room_loc, last_ambient_lux(), last_read_epoch_ms(), "000000");
  }
  return std::string(mqtt_msg);
}
// End ambient light sensor