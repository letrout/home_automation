// Adafruit FunHouse in MBR

#include <Adafruit_DotStar.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <Wire.h>
#include "funhouse_mbr.h"
#include "secrets.h"

#define NUM_DOTSTAR 5
#define BG_COLOR ST77XX_BLACK
#define ALT_M 285 // altitude in meters, for SCD-4x calibration
#define PEPPER_PLANTS 4 // number of pepper plants to monitor

// LEDs!
Adafruit_DotStar pixels(NUM_DOTSTAR, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLOCK, DOTSTAR_BRG);

// Sensors
uint16_t ambient_light;
float prim_temp_f, prim_hum;  // primary temp and humidity measurements

// timers
const unsigned long display_ms = 10000; // display on for x ms after UP button push
unsigned long up_pressed_ms = 0;  // last time UP button pressed
const unsigned long mqtt_ms = 60000;  // publish to mqtt every x ms
unsigned long mqtt_last_ms = 0;
const unsigned long sensor_ms = 1000;  // read sensors every x ms
unsigned long sensor_last_ms = 0;
const unsigned long scd4x_ms = 5000; // read SCD4x sensors every x ms

uint8_t LED_dutycycle = 0;
uint16_t firstPixelHue = 0;
uint8_t pixel_bright;
const uint8_t tft_line_step = 20; // number of pixels in each tft line of text 
bool has_sht4x = false;
bool has_scd4x = false;
bool has_sgp30 = false;
const char* measurement = "environment";
const char* plants_topic = "influx/Owens/plants";
uint8_t peppers[PEPPER_PLANTS] = {100, 75, 50, 0}; // store moisture content for four pepper plants

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  uint8_t cursor_y = 0;
  uint8_t retries = 5, i = 0;

  //while (!Serial);
  Serial.begin(115200);
  delay(100);
  
  pixels.begin(); // Initialize pins for output
  pixels.show();  // Turn all LEDs off ASAP
  pixels.setBrightness(255);

  pinMode(BUTTON_DOWN, INPUT_PULLDOWN);
  pinMode(BUTTON_SELECT, INPUT_PULLDOWN);
  pinMode(BUTTON_UP, INPUT_PULLDOWN);

  //analogReadResolution(13);
  
  tft.init(240, 240);                // Initialize ST7789 screen
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on

  tft.fillScreen(BG_COLOR);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextWrap(false);

  // check DPS!
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("DP310? ");

  Serial.println("Setup DPS310...");
  if (dps.setupDps310()) {  
    tft.setTextColor(ST77XX_RED);
    tft.println("FAIL!");
  }
  tft.setTextColor(ST77XX_GREEN);
  tft.println("OK!");

  // check AHT!
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("AHT20? ");
  
  if (! aht.begin()) {  
    tft.setTextColor(ST77XX_RED);
    tft.println("FAIL!");
    while (1) delay(100);
  }
  tft.setTextColor(ST77XX_GREEN);
  tft.println("OK!");

  // check SHT4x!
  #ifdef ADAFRUIT_SHT4x_H
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("SHT4x? ");
  if (sht4x.setupSht40()) {
    tft.setTextColor(ST77XX_RED);
    tft.println("FAIL!");
  } else {
    has_sht4x = true;
    tft.setTextColor(ST77XX_GREEN);
    tft.println("OK!");
  }
  #endif

  #ifdef SENSIRIONI2CSCD4X_H
  // check SCD-4X!
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("SCD-4X? ");
  retries = 5, i = 0;
  Wire.begin();
  while (i < retries) {
    if (scd4x.setupScd40(ALT_M)) {  
      tft.setTextColor(ST77XX_RED);
      tft.println("FAIL!");
      delay(100);
      i++;
    } else {
      has_scd4x = true;
      tft.setTextColor(ST77XX_GREEN);
      tft.println("OK!");
      break;
    }
  }
  #endif /* SENSIRIONI2CSCD4X_H */

  #ifdef ADAFRUIT_SGP30_H
  // check SGP30!
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("SGP30? ");
  if (sgp30.setupSgp30()) {
    tft.setTextColor(ST77XX_RED);
    tft.println("FAIL!");
  } else {
    has_sgp30 = true;
    tft.setTextColor(ST77XX_GREEN);
    tft.println("OK!");
    // FIXME: move to FhSgp30 class
    Serial.print("Found SGP30 serial #");
    Serial.print(sgp30.serialnumber[0], HEX);
    Serial.print(sgp30.serialnumber[1], HEX);
    Serial.println(sgp30.serialnumber[2], HEX);
  }
  #endif

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SPEAKER, OUTPUT);

  ledcSetup(0, 2000, 8);
  ledcAttachPin(LED_BUILTIN, 0);

  ledcSetup(1, 2000, 8);
  ledcAttachPin(SPEAKER, 1);
  ledcWrite(1, 0);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  retries = 10, i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    i++;
    if (i > retries) {
      break;
    }
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  // Connect to MQTT
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  mqtt_reconnect();
 // get pepper plant data
 client.subscribe(plants_topic);

  // SCD40 needs a few seconds to be ready
  has_scd4x ? delay(5000) : delay(1000);
  tft.fillScreen(BG_COLOR);
} // setup()


void loop() {
  uint8_t cursor_y = 0;

  // timers
  unsigned long now = millis();
  bool sensors_update = false;
  bool mqtt_pubnow = false;

  // check timers
  if ((now - sensor_last_ms) > sensor_ms) {
    sensors_update = true;
    sensor_last_ms = now;
    read_sensors();
  } else {
    sensors_update = false;
  }
  #ifdef SENSIRIONI2CSCD4X_H
  // Update SCD40
  if ((now - scd4x.last_read_ms()) > scd4x_ms) {
    uint16_t scd4x_error = scd4x.readScd40();
    if (scd4x_error) {
      // tft.print("error ");
      // tft.print(scd4x_error, 0);
      Serial.printf("SCD4x error %s\n", scd4x_error);
    } else {
      Serial.printf("SCD40: %d CO2 ppm %0.1f *F  %0.2f rH\n", scd4x.last_co2_ppm(), scd4x.last_temp_f(), scd4x.last_hum_pct());
#ifndef ADAFRUIT_SHT4x_H
      prim_temp_f = scd4x.last_temp_f();
      prim_hum = scd4x.last_hum_pct();
#endif
    }
  }
  #endif /* SENSIRIONI2CSCD4X_H */

  // If UP pressed, display for display_ms ms
  if (digitalRead(BUTTON_UP)) {
    Serial.println("UP pressed");
    // tone(SPEAKER, 1319, 200); // tone1 - E6
    // tone(SPEAKER, 988, 100);  // tone2 - B5
    // delay(100);
    up_pressed_ms = now;
  }
  if ((now - up_pressed_ms) < display_ms) {
    digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on
    cursor_y = display_sensors(cursor_y);
  } else {
    digitalWrite(TFT_BACKLIGHT, LOW); // Backlight off
  }

  // MQTT publish interval expired?
  now = millis();
  if ((now - mqtt_last_ms) > mqtt_ms) {
    mqtt_pubnow = true;
    mqtt_pub_sensors();
    mqtt_last_ms = now;
  } else {
    client.loop();
  }

  /****************** BUTTONS */
  /************* remove to make room for our other sensors *****
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("Buttons: ");
  if (! digitalRead(BUTTON_DOWN)) {  
    tft.setTextColor(0x808080);
  } else {
    Serial.println("DOWN pressed");
    tft.setTextColor(ST77XX_WHITE);
  }
  tft.print("DOWN ");

  if (! digitalRead(BUTTON_SELECT)) {  
    tft.setTextColor(0x808080);
  } else {
    Serial.println("SELECT pressed");
    tft.setTextColor(ST77XX_WHITE);
  }
  tft.print("SEL ");
  
  if (! digitalRead(BUTTON_UP)) {  
    tft.setTextColor(0x808080);
  } else {
    Serial.println("UP pressed");
    tft.setTextColor(ST77XX_WHITE);
  }
  tft.println("UP");
****************************** */

  /************************** CAPACITIVE */
  /************* remove to make room for our other sensors *****
  uint16_t touchread;
  
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("Captouch 6: ");
  touchread = touchRead(6);
  if (touchread < 10000 ) {  
    tft.setTextColor(0x808080, BG_COLOR);
  } else {
    tft.setTextColor(ST77XX_WHITE, BG_COLOR);
  }
  tft.print(touchread);
  tft.println("          ");
  Serial.printf("Captouch #6 reading: %d\n", touchread);
  
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("Captouch 7: ");
  touchread = touchRead(7);
  if (touchread < 20000 ) {  
    tft.setTextColor(0x808080, BG_COLOR);
  } else {
    tft.setTextColor(ST77XX_WHITE, BG_COLOR);
  }
  tft.print(touchread);
  tft.println("          ");
  Serial.printf("Captouch #7 reading: %d\n", touchread);


  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("Captouch 8: ");
  touchread = touchRead(8);
  if (touchread < 20000 ) {  
    tft.setTextColor(0x808080, BG_COLOR);
  } else {
    tft.setTextColor(ST77XX_WHITE, BG_COLOR);
  }
  tft.print(touchread);
  tft.println("          ");
  Serial.printf("Captouch #8 reading: %d\n", touchread);
  ************************************* */


  /************************** ANALOG READ */
  uint16_t analogread;

  /* ******************************************
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("Analog 0: ");
  analogread = analogRead(A0);
  if (analogread < 8000 ) {  
    tft.setTextColor(ST77XX_WHITE, BG_COLOR);
  } else {
    tft.setTextColor(ST77XX_RED, BG_COLOR);
  }
  tft.print(analogread);
  tft.println("    ");
  Serial.printf("Analog A0 reading: %d\n", analogread);


  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("Analog 1: ");
  analogread = analogRead(A1);
  if (analogread < 8000 ) {  
    tft.setTextColor(ST77XX_WHITE, BG_COLOR);
  } else {
    tft.setTextColor(ST77XX_RED, BG_COLOR);
  }
  tft.print(analogread);
  tft.println("    ");
  Serial.printf("Analog A1 reading: %d\n", analogread);

  
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("Analog 2: ");
  analogread = analogRead(A2);
  if (analogread < 8000 ) {  
    tft.setTextColor(ST77XX_WHITE, BG_COLOR);
  } else {
    tft.setTextColor(ST77XX_RED, BG_COLOR);
  }
  tft.print(analogread);
  tft.println("    ");
  Serial.printf("Analog A2 reading: %d\n", analogread);
  ****************************** */
  
  /************************** Beep! */
  if (digitalRead(BUTTON_SELECT)) {  
     Serial.println("** Beep! ***");
     tone(SPEAKER, 988, 100);  // tone1 - B5
     tone(SPEAKER, 1319, 200); // tone2 - E6
     delay(100);
     //tone(SPEAKER, 2000, 100);
  }
  
  /************************** LEDs */
  // pulse red LED
  ledcWrite(0, LED_dutycycle);
  LED_dutycycle += 32;
  
  // rainbow dotstars
  // dim dotstars as ambient light decreases
  pixel_bright = map(ambient_light, 0, 8192, 0, 255);
  /*
  for (int i=0; i<pixels.numPixels(); i++) { // For each pixel in strip...
      if (has_scd4x && (i == 2)) { // third pixel will use CO2 for hue
        continue;
      }
      int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
  }
  */
  // Set dotstars to pepper plant moisture (from MQTT)
  uint16_t pepper_hues[PEPPER_PLANTS];
  for (int i=0; i < PEPPER_PLANTS; i++) {
    pepper_hues[i] = map(peppers[i], 0, 100, 26000, 0); // 0=red, 100=blue
    Serial.print("wet_pct: ");
    Serial.print(peppers[i]);
    Serial.print(", hue: ");
    Serial.println(pepper_hues[i]);
  }
  pixels.setPixelColor(0, pixels.gamma32(pixels.ColorHSV(pepper_hues[3], 255, pixel_bright)));
  pixels.setPixelColor(1, pixels.gamma32(pixels.ColorHSV(pepper_hues[2], 255, pixel_bright)));
  pixels.setPixelColor(3, pixels.gamma32(pixels.ColorHSV(pepper_hues[1], 255, pixel_bright)));
  pixels.setPixelColor(4, pixels.gamma32(pixels.ColorHSV(pepper_hues[0], 255, pixel_bright)));
  /*
  pixels.setPixelColor(0, pixels.ColorHSV(pepper_hues[3]));
  pixels.setPixelColor(1, pixels.ColorHSV(pepper_hues[2]));
  pixels.setPixelColor(3, pixels.ColorHSV(pepper_hues[1]));
  pixels.setPixelColor(4, pixels.ColorHSV(pepper_hues[0]));
  */

#ifdef SENSIRIONI2CSCD4X_H
  // Set middle dotstar hue by CO2 level
    uint16_t co2_hue;
    co2_hue = map(scd4x.last_co2_ppm(), 400, 4000, 0, 26000);  // 400ppm=green, 4000ppm=red
    pixels.setPixelColor(2, pixels.gamma32(pixels.ColorHSV(co2_hue, 255, pixel_bright)));
    Serial.print("CO2 pixel hue ");
    Serial.println(co2_hue);
#endif
  // pixels.setBrightness(pixel_bright);
  pixels.show(); // Update strip with new contents
  firstPixelHue += 256;

  delay(1000);
} // loop()


void read_sensors() {
  // DPS310
  if (!dps.readDps310()) {
    prim_temp_f = dps.last_temp_f();
    Serial.printf("DPS310: %0.1f *F  %0.2f hPa\n", dps.last_temp_f(), dps.last_press_hpa());
  }
  // AHT20
 if (!aht.readAht20()) {
   prim_temp_f = aht.last_temp_f();
   prim_hum = aht.last_hum_pct();
 }
  Serial.printf("AHT20: %0.1f *F  %0.2f rH\n", aht.last_temp_f(), aht.last_hum_pct());
  // Light sensor
  ambient_light = analogRead(A3);
  Serial.printf("Light sensor reading: %d\n", ambient_light);
  // SHT40
  #ifdef ADAFRUIT_SHT4x_H
  sht4x.readSht40();
  prim_temp_f = sht4x.last_temp_f();
  prim_hum = sht4x.last_hum_pct();
  Serial.printf("SHT40: %0.1f *F  %0.2f rH\n", sht4x.last_temp_f(), sht4x.last_hum_pct());
  #endif
  #ifdef ADAFRUIT_SGP30_H
  sgp30.readSgp30(TEMP_C(prim_temp_f), prim_hum);
  Serial.printf("SGP30: %d eCO2 ppm  %d tvoc ppb \n", sgp30.last_eco2(), sgp30.last_tvoc());
  Serial.printf("SGP30: %d ethanol ppm  %d H2 ppm \n", sgp30.last_raw_ethanol(), sgp30.last_raw_h2());
  #endif

  return;
}


void tone(uint8_t pin, float frequency, float duration) {
  ledcSetup(1, frequency, 8);
  ledcAttachPin(pin, 1);
  ledcWrite(1, 128);
  delay(duration);
  ledcWrite(1, 0);
}


void printUint16Hex(uint16_t value) {
  Serial.print(value < 4096 ? "0" : "");
  Serial.print(value < 256 ? "0" : "");
  Serial.print(value < 16 ? "0" : "");
  Serial.print(value, HEX);
}


void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2) {
  Serial.print("Serial: 0x");
  printUint16Hex(serial0);
  printUint16Hex(serial1);
  printUint16Hex(serial2);
  Serial.println();
}


void callback(char *topic, byte *payload, unsigned int length) {
  char* pch;
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
  if (pch = strstr(topic, plants_topic)) {
    get_pepper_mqtt(payload, length);
  }
}


uint8_t display_sensors(const uint8_t cursor_y_start) {
  uint8_t cursor_y = cursor_y_start;
  const uint8_t tft_line_step = 20;

  // DPS310
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("DP310: ");
  tft.print(dps.last_temp_f(), 0);
  tft.print(" F ");
  tft.print(dps.last_press_hpa(), 0);
  tft.print(" hPa");
  tft.println("              ");

  // AHT20
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("AHT20: ");
  tft.print(aht.last_temp_f(), 0);
  tft.print(" F ");
  tft.print(aht.last_hum_pct(), 0);
  tft.print(" %");
  tft.println("              ");

  // SHT40
  #ifdef ADAFRUIT_SHT4x_H
    tft.setCursor(0, cursor_y);
    cursor_y += tft_line_step;
    tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
    tft.print("SHT40: ");
    tft.print(sht4x.last_temp_f(), 0);
    tft.print(" F ");
    tft.print(sht4x.last_hum_pct(), 0);
    tft.print(" %");
    tft.println("              ");
  #endif

  #ifdef SENSIRIONI2CSCD4X_H
  // SCD40
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("SCD4x: ");
  tft.print(scd4x.last_co2_ppm(), 0);
  tft.print(" ppm ");
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.print("SCD4x: ");
  tft.print(scd4x.last_temp_f(), 0);
  tft.print(" F ");
  tft.print(scd4x.last_hum_pct(), 0);
  tft.print(" %");
  tft.println("              ");
  #endif

  // SGP30
  #ifdef ADAFRUIT_SGP30_H
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("SGP30: ");
  tft.print("TVOC ");
  tft.print(sgp30.last_tvoc(), 0);
  tft.print(" ppb ");
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.print("eCO2 ");
  tft.print(sgp30.last_eco2(), 0);
  tft.print(" ppm");

  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("H2 ");
  tft.print(sgp30.last_raw_h2(), 0);
  tft.print(" Eth ");
  tft.print(sgp30.last_raw_ethanol(), 0);
  #endif

  // Light sensor
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("Light: ");
  tft.setTextColor(ST77XX_WHITE, BG_COLOR);
  tft.print(ambient_light);
  tft.println("    ");

  // Pepper plant soil moisture (from MQTT)
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("Pepper:");
  for (int i=0; i < PEPPER_PLANTS; i++) {
    tft.print(" ");
    tft.print(peppers[i]);
  }
  //tft.println("");

  return cursor_y;
}


void mqtt_pub_sensors() {
  char mqtt_msg [128];

  // check/reconnect connection to broker
  mqtt_reconnect();

  // DPS310
  sprintf(mqtt_msg, "%s,sensor=DPS310 temp_f=%f,pressure=%f", measurement, dps.last_temp_f(), dps.last_press_hpa());
  client.publish(topic, mqtt_msg);
  memset(mqtt_msg, 0, sizeof mqtt_msg);
  // AHT20
  sprintf(mqtt_msg, "%s,sensor=AHT20 temp_f=%f,humidity=%f", measurement, aht.last_temp_f(), aht.last_hum_pct());
  client.publish(topic, mqtt_msg);
  memset(mqtt_msg, 0, sizeof mqtt_msg);
  // Ambient light
  sprintf(mqtt_msg, "%s,sensor=funhouse light=%d", measurement, ambient_light);
  client.publish(topic, mqtt_msg);
  memset(mqtt_msg, 0, sizeof mqtt_msg);
  // SHT40
  #ifdef ADAFRUIT_SHT4x_H
  sprintf(mqtt_msg, "%s,sensor=SHT40 temp_f=%f,humidity=%f", measurement, sht4x.last_temp_f(), sht4x.last_hum_pct());
  client.publish(topic, mqtt_msg);
  memset(mqtt_msg, 0, sizeof mqtt_msg);
  #endif
  // SCD40
  if (has_scd4x) {
    sprintf(mqtt_msg, "%s,sensor=SCD40 co2=%d,temp_f=%f,humidity=%f", measurement, scd4x.last_co2_ppm(), scd4x.last_temp_f(), scd4x.last_hum_pct());
    client.publish(topic, mqtt_msg);
    memset(mqtt_msg, 0, sizeof mqtt_msg);
  }
  // SGP30
  #ifdef ADAFRUIT_SGP30_H
  sprintf(mqtt_msg, "%s,sensor=SGP30 tvoc=%d,eco2=%d", measurement, sgp30.last_tvoc(), sgp30.last_eco2());
  client.publish(topic, mqtt_msg);
  memset(mqtt_msg, 0, sizeof mqtt_msg);
  sprintf(mqtt_msg, "%s,sensor=SGP30 h2=%d,ethanol=%d", measurement, sgp30.last_raw_h2(), sgp30.last_raw_ethanol());
  client.publish(topic, mqtt_msg);
  memset(mqtt_msg, 0, sizeof mqtt_msg);
  #endif

  return;
}


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


void mqtt_reconnect() {
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public emqx mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
 }
}