#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <string.h>

// Use appropriate header file for the location
#include "garage.h"
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

int side_door_state = -1; // 0 - closed, 1 - open
int main_door_state = -1; // 0 - closed, 1 - open
int side_door_last = -1; // 0 - closed, 1 - open
int main_door_last = -1; // 0 - closed, 1 - open
const long utcOffsetInSeconds = 0;
const unsigned long ntp_update_ms = 30 * 60 * 1000L; // NTP update interval ms
unsigned long ntp_last_ms = 0L;
char client_id[16] = "d1-"; // will be the MQTT client ID, after MAC appended
const char* measurement = "sensor";
const char* msmt_type = "door";
const char* topic = "influx/Owens/events/doors";
const char* topic_infra = "influx/Owens/infra";

// timers
const unsigned long heartbeat_ms = 10 * 1000; // interval to publish events with no state change
const unsigned long publish_ms = 1 * 1000;  // interval to publish on state change
unsigned long last_publish_side = 0; // last time we did MQTT publish side door
unsigned long last_publish_main = 0; // last time we did MQTT publish main door
const unsigned long publish_infra_ms = 60 * 1000;  // interval to publish infra data
unsigned long last_publish_infra = 0; // last time we did infra MQTT publish

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntp_server, utcOffsetInSeconds);

void setup()
{
  Serial.begin(115200);
  Serial.println();

  pinMode(side_door_pin, INPUT_PULLUP);
  pinMode(main_door_pin, INPUT_PULLUP);

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

  //now = timeClient.getEpochTime();
  side_door_state = digitalRead(side_door_pin);
  main_door_state = digitalRead(main_door_pin);
  if (side_door_state == HIGH) {
    Serial.println("Side door open");
  } else {
    Serial.println("Side door closed");
  }
  if (main_door_state == HIGH) {
    Serial.println("Main door open");
  } else {
    Serial.println("Main door closed");
  }

  // MQTT publish all door states (even if unchanged)
  // message in influxdb2 line protocol format
  // TODO: better way to combine the code for multiple doors?
  // side door
  if (side_door_state != side_door_last) {
    if (mqtt_pub_side(side_door_state)) {
      side_door_last = side_door_state;
      last_publish_side = now;
    } else {
      Serial.println("FAIL to publish side door state");
    }
  } else if ((now - last_publish_side) > heartbeat_ms) {
    if (mqtt_pub_side(side_door_state)) {
      last_publish_side = now;
    } else {
      Serial.println("FAIL to HB publish side door state");
    }
  }
  // main door
  if (main_door_state != main_door_last) {
    if (mqtt_pub_main(main_door_state)) {
      main_door_last = main_door_state;
      last_publish_main = now;
    } else {
      Serial.println("FAIL to publish main door state");
    }
  } else if ((now - last_publish_main) > heartbeat_ms) {
    if (mqtt_pub_main(main_door_state)) {
      last_publish_main = now;
    } else {
      Serial.println("FAIL to HB publish main door state");
    }
  }

  // publish infra?
  if ((now - last_publish_infra) > publish_infra_ms) {
    if (mqtt_pub_wifi()) {
      last_publish_infra = now;
    } else {
      Serial.println("FAIL to publish WiFi RSSI");
    }
  }

  client.loop();
  delay(1000);
} // loop()

boolean mqtt_pub_side(int state) {
  char mqtt_msg [128];
  sprintf(mqtt_msg, "%s,location=%s,room=%s,room_loc=%s,type=%s state=%d %lu%s",
            measurement, location, room, "side", msmt_type, state, timeClient.getEpochTime(), "000000000");
  int len = strlen(mqtt_msg);
  mqtt_reconnect();
  return client.publish(topic, (uint8_t*)mqtt_msg, len, false);
}

boolean mqtt_pub_main(int state) {
  char mqtt_msg [128];
  sprintf(mqtt_msg, "%s,location=%s,room=%s,room_loc=%s,type=%s state=%d %lu%s",
            measurement, location, room, "main", msmt_type, state, timeClient.getEpochTime(), "000000000");
  int len = strlen(mqtt_msg);
  mqtt_reconnect();
  return client.publish(topic, (uint8_t*)mqtt_msg, len, false);
}

boolean mqtt_pub_wifi() {
  char mqtt_msg [128];
  sprintf(mqtt_msg, "wifi,location=%s,room=%s,room_loc=%s,ssid=%s,host=%s rssi=%d %lu%s",
            location, room, "side", WiFi.SSID().c_str(), WiFi.hostname().c_str(), WiFi.RSSI(), timeClient.getEpochTime(), "000000000");
  int len = strlen(mqtt_msg);
  mqtt_reconnect();
  return client.publish(topic_infra, (uint8_t*)mqtt_msg, len, false);
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
      delay(1000);
    }
  }
}
