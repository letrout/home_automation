#include "ambient_light.h"


// Ambient light sensor
AmbientLight::AmbientLight(void) {
}

void AmbientLight::read(void) {
  if (measurementReady()) {
    last_ambient_lux_ = readLightLevel();
    last_read_ms_ = millis();
  }
  return;
}
// End ambient light sensor