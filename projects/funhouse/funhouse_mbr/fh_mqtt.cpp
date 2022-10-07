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
#include "fh_homesec.h"
#include "secrets.h"

#define WIFI_RETRIES 10
const uint16_t fh_mqtt_buffer_size = 512;  //set the MQTT buffer size (see setBufferSize())

FhWifi fh_wifi;
WiFiClient espClient;
FhPubSubClient mqtt_client;

#ifdef FH_SUB_PEPPERS
uint8_t peppers[PEPPER_PLANTS] = {100, 75, 50, 0}; // store moisture content for four pepper plants
#endif

#ifdef FH_HOMESEC_H
extern const char* doors_topic;
#endif

// Wifi
FhWifi::FhWifi(void) {
    // connect();
}

uint8_t FhWifi::connect(void) {
    uint8_t retval = 1;
    uint8_t i = 0;
    mode(WIFI_STA);
    // ssid, password, net_hostname from secrets.h
    begin(wifi_ssid, wifi_password);
    setHostname(net_hostname);
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
    //setBufferSize(fh_mqtt_buffer_size);
}

int FhPubSubClient::publishTopic(const char *payload) {
    return publish(topic, payload);
}

void FhPubSubClient::mqttReconnect(void) {
    while (!connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("Client-id %s, connect...\n", client_id.c_str());
        if (connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("MQTT broker connected");
#ifdef FH_SUB_PEPPERS
            // get pepper plant data
            mqtt_client.subscribe(topic_plants);
#endif
#ifdef FH_HOMESEC_H
            //mqtt_client.subscribe(doors_topic, 1);
            mqtt_client.subscribe(doors_topic);
#endif
        } else {
            Serial.print("failed with state ");
            Serial.print(state());
            delay(2000);
        }
    }
}

#ifdef FH_SUB_PEPPERS
int8_t get_pepper_mqtt(const byte* payload, const int length) {
  int8_t ret = -1;
  char msg[length];
  char *pch;
  uint8_t pepper_number;
  //memccpy(msg, payload, sizeof(payload), sizeof(char));
  for (int i = 0; i < length; i++) {
    msg[i] = (char)payload[i];
  }

  // Get the pepper number
  pch = strstr(msg, "plant"); // payload starting at "plant"
  if (pch != NULL) {
    pch = strtok(pch, "=, "); // split result on delimiters
  }
  if (pch != NULL) {
    pch = strtok(NULL, "=pepper, ");  // get the second token after split
  }
  if ((pch == NULL) || (pch[0] =='\0')) {
    return 1;
  }
  pepper_number = atoi(pch);
  if (pepper_number > PEPPER_PLANTS) {
    return 2;
  }
  // Get the wet %
  for (int i = 0; i < length; i++) {
    msg[i] = (char)payload[i];
  }
  pch = strstr(msg, "wet_pct"); // payload starting at value name
  if (pch != NULL) {
    pch = strtok(pch, "=, "); // split result on delimiters
  }
  if (pch != NULL) {
    pch = strtok(NULL, "=, ");  // get the second token after split
  }
  if ((pch == NULL) || (pch[0] =='\0')) {
    ret = 3;
  } else {
    peppers[pepper_number - 1] = atoi(pch);
    ret = 0;
    Serial.print("pepper ");
    Serial.print(pepper_number);
    Serial.print(": ");
    Serial.println(peppers[pepper_number - 1]);
  }
  return ret;
}
#endif