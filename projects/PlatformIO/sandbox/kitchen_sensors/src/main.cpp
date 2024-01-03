#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <string.h>
#include <Wire.h>

#include "door_monitor.h"
// Use appropriate header file for the location
#include "kitchen.h"
#include "ambient_light.h"
#include "Luth_SHT30.h"
#include "Luth_SGP30.h"
#include "door.h"
#include "pir.h"
#include "owens_sensors.h"
#include "secrets.h"

#define SERIAL_DEBUG
#define MQTT_TEST

#define ARRAY_LENGTH(array) (sizeof(array)/sizeof((array)[0]))

// From secrets.h
const char* ssid     = STASSID;
const char* password = STAPSK;
const char* ntp_server = NTP_SERVER;
const char* mqtt_broker = MQTT_BROKER;
const int mqtt_port = MQTT_PORT;
const char* mqtt_username = MQTT_USER;
const char* mqtt_password = MQTT_PASSWORD;

int door_last_state = -1; // initialize to invalid state
int pir_last_state = -1; // initialize to invalid state
const long utcOffsetInSeconds = 0;
const unsigned long ntp_update_ms = 30 * 60 * 1000L; // NTP update interval ms
unsigned long ntp_last_ms = 0L;
char client_id[16] = "d1-"; // will be the MQTT client ID, after MAC appended
#ifdef MQTT_TEST
const char* door_topic = "influx/Owens/test";
const char* motion_topic = "influx/Owens/test";
const char* infra_topic = "influx/Owens/test";
const char* env_topic = "influx/Owens/test";
#else
const char* door_topic = "influx/Owens/events/doors";
const char* motion_topic = "influx/Owens/events/motion";
const char* infra_topic = "influx/Owens/infra";
const char* env_topic = "influx/Owens/sensors";
#endif

// timers
const unsigned long event_heartbeat_ms = 10 * 1000; // interval to publish events with no state change
const unsigned long event_publish_ms = 1 * 1000;  // interval to publish on event state change
unsigned long door_last_publish = 0; // last time we did door event MQTT publish
unsigned long pir_last_publish = 0; // last time we did PIR event MQTT publish
const unsigned long env_publish_ms = 60 * 1000;  // interval to publish environmental data
unsigned long env_last_publish = 0; // last time we did environmental MQTT publish

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntp_server, utcOffsetInSeconds);

// sensors
DoorSensor deckDoor(door_pin, location, room, room_loc);
#ifdef PIR_MOTION_H
PirSensor deckPir(pir_pin, location, room, room_loc);
#endif
#ifdef AMBIENT_LIGHT_H
AmbientLight lightMeter(location, room, room_loc, 0x23);
#endif
#ifdef LUTH_SHT30_H
LuthSht30 sht30(location, room, room_loc, 0x45);
#endif
#ifdef LUTH_SGP30_H
LuthSgp30 sgp30(location, room, room_loc);
#endif


void setup()
{
  Serial.begin(115200);
  Serial.println();

  Wire.begin();

  deckDoor.begin();

#ifdef PIR_MOTION_H
  deckPir.begin();
#endif

#ifdef AMBIENT_LIGHT_H
  // begin returns a boolean that can be used to detect setup problems.
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
#ifdef SERIAL_DEBUG
    Serial.println(F("BH1750 Advanced begin"));
#endif
  } else {
    Serial.println(F("Error initialising BH1750"));
  }
#endif

#ifdef LUTH_SHT30_H
  if (sht30.begin()) {
#ifdef SERIAL_DEBUG
    Serial.println(F("SHT30 Advanced begin"));
#endif
  } else {
    Serial.println(F("Error initialising SHT30"));
  }
#endif
#ifdef LUTH_SGP30_H
  if (sgp30.begin()) {
    Serial.print("TEST:\t");
    Serial.println(sgp30.measureTest());
    Serial.print("FSET:\t");
    Serial.println(sgp30.getFeatureSet(), HEX);
    sgp30.GenericReset();
#ifdef SERIAL_DEBUG
    Serial.println(F("SGP30 Advanced begin"));
#endif
  } else {
    Serial.println(F("Error initialising SGP30"));
  }
#endif

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  // NTP
  timeClient.begin();
  // Not sure why, but seem to need a little delay before updating time
  delay(1000);
  timeClient.update();
  ntp_last_ms = millis();

  //MQTT
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  // append our MAC to client_id so it's unique
  byte bmac[6];
  WiFi.macAddress(bmac);
  for (byte i = 0; i < ARRAY_LENGTH(bmac); ++i) {
    char buf[3];
    sprintf(buf, "%02x", bmac[i]);
    strncat(client_id, buf, 3);
  }
  mqtt_reconnect();
} // setup()

void loop() {
  unsigned long now = millis();
  // char mqtt_msg [128];
  if ((now - ntp_last_ms) > ntp_update_ms) {
    ntp_last_ms = now;
    timeClient.update();
  }
  //print_time();

#ifdef PIR_MOTION_H
  if (deckPir.read() == HIGH) {
#ifdef SERIAL_DEBUG
    Serial.print("PIR triggered: ");
#endif
  } else {
    Serial.print("PIR not triggered: ");
  }
  Serial.println(millis());
   // MQTT publish all PIR states (even if unchanged)
  // message in influxdb2 line protocol format
  if (deckPir.last_read_state() != pir_last_state) {
#ifdef SERIAL_DEBUG
    Serial.println("PIR state changes, publishing...");
#endif
    if (deckPir.mqtt_pub(client, motion_topic)) {
      pir_last_state = deckPir.last_read_state();
      pir_last_publish = now;
#ifdef SERIAL_DEBUG
      Serial.println("PIR state published");
#endif
    } else {
      Serial.println("FAIL to publish PIR state");
    }
  } else if ((now - pir_last_publish) > event_heartbeat_ms) {
#ifdef SERIAL_DEBUG
    Serial.println("PIR heartbeat expired, publishing...");
#endif
    if (deckPir.mqtt_pub(client, motion_topic)) {
#ifdef SERIAL_DEBUG
      Serial.println("PIR state published");
#endif
      pir_last_publish = now;
    } else {
      Serial.println("FAIL to publish PIR state");
    }
  }
#endif

  deckDoor.read();
  #ifdef SERIAL_DEBUG
  if (deckDoor.last_read_state() == HIGH) {
    Serial.print("Door open: ");
  } else {
    Serial.print("Door closed: ");
  }
  Serial.println(millis());
  #endif

  // MQTT publish all door states (even if unchanged)
  // message in influxdb2 line protocol format
  if (deckDoor.last_read_state() != door_last_state) {
  #ifdef SERIAL_DEBUG
    Serial.println("Door state changes, publishing...");
  #endif
    if (deckDoor.mqtt_pub(client, door_topic)) {
      door_last_state = deckDoor.last_read_state();
      door_last_publish = now;
  #ifdef SERIAL_DEBUG
      Serial.println("Door state published");
  #endif
    } else {
      Serial.println("FAIL to publish door state");
    }
  } else if ((now - door_last_publish) > event_heartbeat_ms) {
  #ifdef SERIAL_DEBUG
    Serial.println("Door heartbeat expired, publishing...");
  #endif
    if (deckDoor.mqtt_pub(client, door_topic)) {
  #ifdef SERIAL_DEBUG
      Serial.println("Door state published");
  #endif
      door_last_publish = now;
    } else {
      Serial.println("FAIL to publish door state");
    }
  }

  // Environment seonsors
#ifdef AMBIENT_LIGHT
  if (lightMeter.read() == E_SENSOR_SUCCESS) {
#ifdef SERIAL_DEBUG
    Serial.print("Light: ");
    Serial.print(lightMeter.last_ambient_lux());
    Serial.println(" lx");
#endif
    if (millis() - lightMeter.last_publish_ms() > env_publish_ms) {
      lightMeter.mqtt_pub(client, env_topic);
    }
  }
#endif // AMBIENT_LIGHT

#ifdef LUTH_SHT30_H
  uint16_t sht30_status = sht30.read();
  if (sht30_status == E_SENSOR_SUCCESS) {
    if (millis() - sht30.last_publish_ms() > env_publish_ms) {
      sht30.mqtt_pub(client, env_topic);
    }
#ifdef SERIAL_DEBUG
    Serial.print("Temp F: ");
    Serial.print(sht30.last_temp_f());
    Serial.print(", Humidity: ");
    Serial.println(sht30.last_hum_rel());
#endif
  } else if (sht30_status == E_SENSOR_NOOP) {
#ifdef SERIAL_DEBUG
    Serial.println("SHT30: no new data");
#endif
  } else {
    Serial.print("Error reading SHT30: ");
    Serial.print(sht30_status, HEX);
    Serial.println();
  }
#endif // LUTH_SHT30_H

#ifdef LUTH_SGP30_H
  uint16_t sgp30_status = sgp30.read();
  if (sgp30_status == E_SENSOR_SUCCESS) {
    if (millis() - sgp30.last_publish_ms() > env_publish_ms) {
      sgp30.mqtt_pub(client, env_topic);
    }
#ifdef SERIAL_DEBUG
    Serial.print("eCO2: ");
    Serial.print(sgp30.last_eco2());
    Serial.print(", TVOC: ");
    Serial.println(sgp30.last_tvoc());
#endif
  } else if (sgp30_status == E_SENSOR_NOOP) {
#ifdef SERIAL_DEBUG
    Serial.println("SGP30: no new data");
#endif
  } else {
    Serial.println("Failed to read SGP30");
  }
#endif // LUTH_SGP30_H

  // publish infra?
  if ((now - env_last_publish) > env_publish_ms) {
    if (mqtt_pub_wifi()) {
      env_last_publish = now;
    } else {
      Serial.println("FAIL to publish WiFi RSSI");
    }
  }

  client.loop();
  delay(event_publish_ms);
} // loop()

boolean mqtt_pub_wifi() {
  char mqtt_msg [128];
  sprintf(mqtt_msg, "wifi,location=%s,room=%s,room_loc=%s,ssid=%s,host=%s rssi=%d %lu%s",
            location, room, room_loc, WiFi.SSID().c_str(), WiFi.hostname().c_str(), WiFi.RSSI(), timeClient.getEpochTime(), "000000000");
  int len = strlen(mqtt_msg);
  mqtt_reconnect();
  return client.publish(infra_topic, (uint8_t*)mqtt_msg, len, false);
}

void print_time() {
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.println(timeClient.getSeconds());
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
  
  /* example: extract the "temp_f" field from mqtt msg */
  /*
  float value;
  int ret;
  ret = get_mqtt_val("temp_f", payload, length, &value);
  if (ret == 0) {
    Serial.print("value: ");
    Serial.println(value);
  }
  */
}

void mqtt_reconnect() {
  while (!client.connected()) {
    Serial.printf("client %s connecting to mqtt broker...\n", client_id);
    if (client.connect(client_id, mqtt_username, mqtt_password)) {
      Serial.println("mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}
