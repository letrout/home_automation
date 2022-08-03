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