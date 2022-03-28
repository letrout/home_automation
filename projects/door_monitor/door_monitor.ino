#include <ESP8266WiFi.h>

#include "secrets.h"

const char* ssid     = STASSID;
const char* password = STAPSK;

void setup()
{
  Serial.begin(115200);
  Serial.println();

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
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  delay(5000);
  // new line
}