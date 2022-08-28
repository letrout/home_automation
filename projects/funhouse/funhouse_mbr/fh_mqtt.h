#ifndef FH_MQTT_H
#define FH_MQTT_H

#include <NTPClient.h>
#include <PubSubClient.h>
#include <WiFi.h>

#define FH_SUB_PEPPERS 1    // Subscribe to pepper plant messages
#define PEPPER_PLANTS 4 // number of pepper plants to monitor

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

class FhNtpClient : NTPClient {
    private:
    public:
        /**
         * @brief Construct a new Fh Ntp Client object
         * uses const char* ntp_server, long utcOffsetInSeconds from secrets.h
         * 
         * @param udp WiFiUDP object
         */
        FhNtpClient(UDP& udp);
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
        FhPubSubClient();
        /**
         * @brief Set up the MQTT client
         * set the WiFi client
         * 
         */
        void setup(void);
        /**
         * @brief Set the MQTT server with domain/port from secrets.h
         * 
         */
        void setMqttServer(void);
        /**
         * @brief Call publish() from thw base class, using topic from secrets.h
         * 
         * @param payload MQTT message payload
         * @return int return value from publish()
         */
        int publishTopic(const char *payload);
        /**
         * @brief Re-connect to the MQTT server, using credentials from secrets.h
         * check if connected to mqtt broker, reconnect loop if necessary
         * This is blocking. Right now this app only exists to publish to MQTT,
         * but if that changes this will need to change to something non-blocking
         * 
         */
        void mqttReconnect(void);
};

#ifdef FH_SUB_PEPPERS
/**
 * @brief Get the pepper plant moisture level from MQTT
 * 
 * @param payload MQTT payload
 * @param length length of payload
 * @return int8_t error code (0 on success)
 */
int8_t get_pepper_mqtt(const byte* payload, const int length);
#endif

#endif /* FH_MQTT_H */