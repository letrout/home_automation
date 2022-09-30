#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <string.h>

#include "door_monitor.h"
// Use appropriate header file for the location
#include "kitchen.h"
#include "secrets.h"

#define ARRAY_LENGTH(array) (sizeof(array)/sizeof((array)[0]))

// From secrets.h
const char* ssid     = STASSID;
const char* password = STAPSK;
const char* ntp_server = NTP_SERVER;
const char* mqtt_broker = MQTT_BROKER;
const int mqtt_port = MQTT_PORT;
const char* mqtt_username = MQTT_USER;
const char* mqtt_password = MQTT_PASSWORD;

int door_state = -1; // 0 - closed, 1 - open
int door_last_state = -1; // initialize to invalid state
const long utcOffsetInSeconds = 0;
const unsigned long ntp_update_ms = 30 * 60 * 1000L; // NTP update interval ms
unsigned long ntp_last_ms = 0L;
char client_id[16] = "d1-"; // will be the MQTT client ID, after MAC appended
const char* measurement = "sensor";
const char* msmt_type = "door";
const char* topic = "influx/Owens/events/doors";

// timers
const unsigned long heartbeat_ms = 10 * 1000; // interval to publish events with no state change
const unsigned long publish_ms = 1 * 1000;  // interval to publish on state change
unsigned long last_publish = 0; // last time we did MQTT publish

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntp_server, utcOffsetInSeconds);

void setup()
{
  Serial.begin(115200);
  Serial.println();

  pinMode(door_pin, INPUT_PULLUP);

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
  char mqtt_msg [128];
  if ((now - ntp_last_ms) > ntp_update_ms) {
    ntp_last_ms = now;
    timeClient.update();
  }
  //print_time();

  door_state = digitalRead(door_pin);
  if (door_state == HIGH) {
    Serial.print("Door open: ");
  } else {
    Serial.print("Door closed: ");
  }
  Serial.println(millis());

  // MQTT publish all door states (even if unchanged)
  // message in influxdb2 line protocol format
  if (door_state != door_last_state) {
    if (mqtt_pub_door(door_state)) {
      door_last_state = door_state;
      last_publish = now;
    } else {
      Serial.println("FAIL to publish door state");
    }
  } else if ((now - last_publish) > heartbeat_ms) {
    if (mqtt_pub_door(door_state)) {
      last_publish = now;
    } else {
      Serial.println("FAIL to publish door state");
    }
  }

  client.loop();
  delay(publish_ms);
} // loop()

boolean mqtt_pub_door(int state) {
  char mqtt_msg [128];
  sprintf(mqtt_msg, "%s,location=%s,room=%s,room_loc=%s,type=%s state=%d %lu%s",
            measurement, location, room, room_loc, msmt_type, door_state, timeClient.getEpochTime(), "000000000");
  int len = strlen(mqtt_msg);
  mqtt_reconnect();
  return client.publish(topic, (uint8_t*)mqtt_msg, len, false);
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
  for (int i = 0; i < length; i++) {
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
