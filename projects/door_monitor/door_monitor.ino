#include <ESP8266WiFi.h>

#include "secrets.h"

const int door_pin = D1;
const char* ssid     = STASSID;
const char* password = STAPSK;

int door_state; // 0 - closed, 1 - open

void setup()
{
  Serial.begin(115200);
  Serial.println();

  pinMode(door_pin, INPUT_PULLUP);

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
}

void loop() {
  // Serial.print("Connected, IP address: ");
  // Serial.println(WiFi.localIP());

  door_state = digitalRead(door_pin);
  if (door_state == HIGH) {
    Serial.println("Door open");
  } else {
    Serial.println("Door closed");
  }

  delay(5000);
  // new line
}
