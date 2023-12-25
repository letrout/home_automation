#ifndef AMBIENT_LIGHT_H
#define AMBIENT_LIGHT_H 1

#include <string>
#include <BH1750.h>
#include "owens_sensors.h"

const char * const AMBIENT_LIGHT_MEASUREMENT = "environment";

class AmbientLight : public BH1750 {
  private:
    float last_ambient_lux_;
    unsigned long last_read_ms_;
    unsigned long last_read_epoch_ms_ = 0;

  public:
    AmbientLight(byte addr = 0x23)
    : BH1750(addr) {
      last_ambient_lux_ = 0.0;
      last_read_ms_ = 0;
    };
    float last_ambient_lux() const { return last_ambient_lux_; }
    unsigned long last_read_ms() const { return last_read_ms_; }
    unsigned long last_read_epoch_ms() const { return last_read_epoch_ms_; }
    /**
     * @brief read the ambient light sensor
     * 
     */
    int8_t read();
    std::string mqtt_msg_lp(
      const char * location,
      const char * room,
      const char * room_loc);
};

#endif // AMBIENT_LIGHT_H