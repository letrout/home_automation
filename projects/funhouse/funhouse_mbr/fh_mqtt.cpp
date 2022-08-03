/**
 * @file fh_mqtt.cpp
 * @author Joel Luth (joel.luth@gmail.com)
 * @brief Classes to manage MQTT on Adafruit Funhouse
 * @version 0.1
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "fh_mqtt.h"
#include "secrets_wifi.h"
#include "secrets.h"

#define WIFI_RETRIES 10

FhWifi fh_wifi;
WiFiClient espClient;
FhPubSubClient client;

// Wifi
FhWifi::FhWifi(void) {
    // connect();
}

uint8_t FhWifi::connect(void) {
    uint8_t retval = 1;
    uint8_t i = 0;
    begin(ssid, password);
    while (status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        i++;
        if (i > WIFI_RETRIES) {
            break;
        }
    }
    // FIXME: I original had this as != WL_CONNECTED??
    if (status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.print("IP: ");
        Serial.println(localIP());
        Serial.print("MAC: ");
        Serial.println(macAddress());
        retval = 0;
    } else {
        return status();
    }
    setAutoReconnect(true);
    persistent(true);
    return retval;
}

// MQTT client
FhPubSubClient::FhPubSubClient(void) {
    // connect();
}

void FhPubSubClient::setup(void) {
    setClient(espClient);
}

void FhPubSubClient::setMqttServer(void) {
    //setServer(mqtt_broker, mqtt_port);
    setServer(mqtt_broker, mqtt_port);
}

int FhPubSubClient::publishTopic(const char *payload) {
    return publish(topic, payload);
}

void FhPubSubClient::mqttReconnect(void) {
    while (!connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
        if (connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public emqx mqtt broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(state());
            delay(2000);
        }
    }
}