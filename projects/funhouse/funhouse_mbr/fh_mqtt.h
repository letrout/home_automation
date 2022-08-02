#ifndef FH_MQTT_H
#define FH_MQTT_H

#include <WiFi.h>

class FhWifi : public WiFiClass {
    private:
    public:
        FhWifi();
        /**
         * @brief connect to WiFi using credentials from secrets.h
         * 
         * @return uint8_t error code, 0 on success
         */
        uint8_t connect();
};

#endif /* FH_MQTT_H */