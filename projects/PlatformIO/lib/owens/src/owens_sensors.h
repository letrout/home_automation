#ifndef OWENS_SENSORS_H
#define OWENS_SENSORS_H 1

#define E_SENSOR_SUCCESS 0  // standard success return value
#define E_SENSOR_FAIL 1  // standard read failure return value
#define E_SENSOR_NOOP 2  // standard no-op return value, (eg, no data from sensor)
#define E_SENSOR_NOT_READY 4 // operation not ready (eg sensor ready timer not expired)
#define E_SENSOR_NOT_PRESENT 8  // standard return value for "sensor not found"
#define BASE_MQTT_TOPIC "influx/Owens"

#endif // OWENS_SENSORS_H