#ifndef FH_MQTT_H
#define FH_MQTT_H

#include <PubSubClient.h>
#include <WiFi.h>

class FhWifi {
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

class FhPubSubClient : public PubSubClient {
    private:
        WiFiClient wifi_client_;
    public:
        /*
        FhPubSubClient(WiFiClient wifi_client)
        : PubSubClient{wifi_client}
        {
            setServer(mqtt_broker, mqtt_port);
            wifi_client_ = wifi_client;
        }
        */
        FhPubSubClient(WiFiClient wifi_client);
        /**
         * @brief Call publish() from thw base class, using topic from secrets.h
         * 
         * @param payload MQTT message payload
         * @return int return value from publish()
         */
        int publishTopic(const char *payload);
        void mqttReconnect(void);
};

#endif /* FH_MQTT_H */