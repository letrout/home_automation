#ifndef AMBIENT_LIGHT_H
#define AMBIENT_LIGHT_H 1

#include <BH1750.h>
#include "owens_sensors.h"

class AmbientLight : public BH1750 {
  private:
    float last_ambient_lux_;
    unsigned long last_read_ms_;

  public:
    AmbientLight();
    float last_ambient_lux() const { return last_ambient_lux_; }
    unsigned long last_read_ms() const { return last_read_ms_; }
    /**
     * @brief read the ambient light sensor
     * 
     */
    void read();
};

#endif // AMBIENT_LIGHT_H