// Adafruit FunHouse in MBR

#include <Wire.h>
#include "funhouse_mbr.h"
#include "fh_dotstar.h"
#include "fh_homesec.h"
#include "fh_mqtt.h"
#include "fh_tft.h"
#include "fh_time.h"

#define NUM_BUTTONS 3
#define ALT_M 285 // altitude in meters, for SCD-4x calibration

// sensors objects
extern FhAmbientLight ambientLight;
extern FhDps310 dps;
extern FhAht20 aht;
#ifdef ADAFRUIT_SGP30_H
extern FhSgp30 sgp30;
#endif
#ifdef ADAFRUIT_SHT4x_H
extern FhSht40 sht4x;
#endif
#ifdef SENSIRIONI2CSCD4X_H
extern FhScd40 scd4x;
#endif
#ifdef ADAFRUIT_PM25AQI_H
extern FhPm25Aqi pm25Aqi;
#endif

// display!
extern FhTft tft;

// LEDs!
extern FhDotstar pixels;

// Sensors
float prim_temp_f, prim_hum;  // primary temp and humidity measurements

// timers
const unsigned long display_ms = 10000; // display on for x ms after UP button push
unsigned long button_pressed_ms[NUM_BUTTONS] = {0};  // last time buttons pressed {UP, SELECT, DOWN} TODO: map?
const unsigned long mqtt_ms = 60000;  // publish to mqtt every x ms
const unsigned long max_mqtt_pub_delay_ms = mqtt_ms; // max age of a measurement to publish, ms (should be <= mqtt_ms)
unsigned long mqtt_last_ms = 0;
const unsigned long sensor_ms = 1000;  // read sensors every x ms
unsigned long sensor_last_ms = 0;
const unsigned long scd4x_ms = 10000; // read SCD4x sensors every x ms, min 5000

uint8_t LED_dutycycle = 0;
const char* measurement = "environment";
#ifdef FH_SUB_PEPPERS
extern const char* topic_plants;
#endif
#ifdef FH_HOMESEC_H
extern const char* doors_topic;
std::map<const char*, OwensDoor, char_cmp> owensDoors;
#endif

extern FhWifi fh_wifi;
// WiFiClient espClient;
extern FhPubSubClient mqtt_client;
extern FhNtpClient ntp_client;
// Location info (from secrets.h)
extern const char* location;
extern const char* room;
extern const char* room_loc;
extern const char* topic_infra;

void setup() {

  // while (!Serial);
  Serial.begin(115200);
  delay(100);

  ntp_client = FhNtpClient(300);
  
  // Initialize the dotstars
  pixels.setup();

  pinMode(BUTTON_DOWN, INPUT_PULLDOWN);
  pinMode(BUTTON_SELECT, INPUT_PULLDOWN);
  pinMode(BUTTON_UP, INPUT_PULLDOWN);

  //analogReadResolution(13);
  
  // Initialize the display
  tft.setup();

  tft.setDisplayMode(DISPLAY_MODE_ALL_SENSORS, true);
  // check DPS!
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
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("SHT4x? ");
  if (sht4x.setupSht40()) {
    tft.setTextColor(ST77XX_RED);
    tft.println("FAIL!");
  } else {
    tft.setTextColor(ST77XX_GREEN);
    tft.println("OK!");
  }
  #endif

  #ifdef SENSIRIONI2CSCD4X_H
  // check SCD-4X!
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("SCD-4X? ");
  Wire.begin();
  if (scd4x.setupScd40(5, ALT_M)) {
    tft.setTextColor(ST77XX_RED);
    tft.println("FAIL!");
  } else {
    tft.setTextColor(ST77XX_GREEN);
    tft.println("OK!");
  }
  #endif /* SENSIRIONI2CSCD4X_H */

  #ifdef ADAFRUIT_SGP30_H
  // check SGP30!
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("SGP30? ");
  if (sgp30.setupSgp30()) {
    tft.setTextColor(ST77XX_RED);
    tft.println("FAIL!");
  } else {
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
  fh_wifi.connect();

  // Connect to MQTT
  mqtt_client.setup();
  mqtt_client.setMqttServer();
  mqtt_client.setCallback(callback);
  mqtt_client.setBufferSize(512);
  //mqtt_client.setSocketTimeout(30);
  mqtt_client.setKeepAlive(70);
  mqtt_client.mqttReconnect();
#ifdef FH_HOMESEC_H
  uint32_t door_last_sec;
  owensDoors = get_doors();
  // Prime the object with last open time from InfluxDB
  // after this, last open time will be maintained via MQTT sub
  for (const auto &door : owensDoors) {
    owensDoors.at(door.first).secLastOpen(&door_last_sec, true);
  }
  // InfluxDB seems to break MQTT connection, so reconnect
  mqtt_client.mqttReconnect();
#endif

#ifdef SENSIRIONI2CSCD4X_H
  // SCD40 needs a few seconds to be ready
  scd4x.present() ? delay(5000) : delay(1000);
#endif
  tft.fillScreen(BG_COLOR);
} // setup()


void loop() {
  // timers
  unsigned long now = millis();
  bool sensors_update = false;
  bool mqtt_pubnow = false;

  mqtt_client.loop();
  delay(100);
  if (mqtt_client.state() != MQTT_CONNECTED) {
    Serial.printf("MQTT state: %d\n", mqtt_client.state());
  }

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
    //uint16_t scd4x_error = scd4x.readScd40();
    uint16_t scd4x_error = scd4x.readScd40(static_cast<uint16_t>(dps.last_press_hpa()));
    if (scd4x_error) {
      // tft.print("error ");
      // tft.print(scd4x_error, 0);
      Serial.printf("SCD4x error %d\n", scd4x_error);
    } else {
      Serial.printf("SCD40: %d CO2 ppm %0.1f *F  %0.2f rH\n", scd4x.last_co2_ppm(), scd4x.last_temp_f(), scd4x.last_hum_pct());
#ifdef ADAFRUIT_SHT4x_H
      if (!sht4x.present()) {
        prim_temp_f = scd4x.last_temp_f();
        prim_hum = scd4x.last_hum_pct();
      }
#else
      prim_temp_f = scd4x.last_temp_f();
      prim_hum = scd4x.last_hum_pct();
#endif
    }
  }
  #endif /* SENSIRIONI2CSCD4X_H */

  // Record button press times
  // TODO: move the fillscreen() to the display_() in the switch below?
  if (digitalRead(BUTTON_UP)) {
    Serial.println("UP pressed");
    // tone(SPEAKER, 1319, 200); // tone1 - E6
    // tone(SPEAKER, 988, 100);  // tone2 - B5
    // delay(100);
    tft.fillScreen(BG_COLOR);
    button_pressed_ms[0] = now;
  }
  if (digitalRead(BUTTON_SELECT)) {
    Serial.println("SELECT pressed");
    // tft.fillScreen(BG_COLOR);
    button_pressed_ms[1] = now;
  }
  if (digitalRead(BUTTON_DOWN)) {
    Serial.println("DOWN pressed");
    tft.fillScreen(BG_COLOR);
    button_pressed_ms[2] = now;
  }

  // Get the last button push
  uint8_t last_button = 0;
  unsigned long max_ms = button_pressed_ms[0];
  // last_button = distance(button_pressed_ms, max_element(button_pressed_ms, button_pressed_ms + NUM_BUTTONS));
  Serial.println("button press loop start...");
  for (uint8_t i=0; i < (sizeof(button_pressed_ms) / sizeof(button_pressed_ms[0])); i++) {
    if (button_pressed_ms[i] > max_ms) {
      max_ms = button_pressed_ms[i];
      last_button = i;
    }
  }
  Serial.print("last button pressed: ");
  Serial.println(last_button);
  // Set the display based on button pushed
  // FIXME: need to add dotstar mode here, as well as TFT mode
  if ((now - button_pressed_ms[last_button]) < display_ms) {
    switch (last_button) {
      case 0:
        // BUTTON_UP - display environmental data
        tft.displayEnvironment();
        pixels.setMode(DOTSTAR_MODE_PLANTS);
        break;
      case 1:
        // BUTTON_SELECT Display door sensor events
        tft.displayDoors();
        pixels.setMode(DOTSTAR_MODE_SLEEP, true);
        break;
      case 2:
        // BUTTON_DOWN - display all sensor data
        tft.displaySensors();
        pixels.setMode(DOTSTAR_MODE_RAINBOW);
        break;
      // default:
    }
  } else {
    tft.setDisplayMode(DISPLAY_MODE_SLEEP);
    pixels.setMode(DOTSTAR_MODE_SLEEP, true);
  }
  Serial.println("done with button section");

  // MQTT publish interval expired?
  now = millis();
  if ((now - mqtt_last_ms) > mqtt_ms) {
    mqtt_pubnow = true;
    mqtt_pub_sensors();
    mqtt_last_ms = now;
  } else {
    // mqtt_client.loop();
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
     fh_tone(SPEAKER, 988, 100);  // tone1 - B5
     fh_tone(SPEAKER, 1319, 200); // tone2 - E6
     delay(100);
     //tone(SPEAKER, 2000, 100);
  }
  
  /************************** LEDs */
  // pulse red LED
  ledcWrite(0, LED_dutycycle);
  LED_dutycycle += 16;

  //delay(500);
} // loop()


void read_sensors() {
  uint8_t sensor_return;
  // DPS310
  if (!dps.readDps310()) {
#if !defined(ADAFRUIT_SHT4x_H) && !defined(SENSIRIONI2CSCD4X_H)
    prim_temp_f = dps.last_temp_f();
#endif
    Serial.printf("DPS310: %0.1f *F  %0.2f hPa\n", dps.last_temp_f(), dps.last_press_hpa());
  }
  // AHT20
 if (!aht.readAht20()) {
#if !defined(ADAFRUIT_SHT4x_H) && !defined(SENSIRIONI2CSCD4X_H)
   prim_temp_f = aht.last_temp_f();
   prim_hum = aht.last_hum_pct();
#endif
 }
  Serial.printf("AHT20: %0.1f *F  %0.2f rH\n", aht.last_temp_f(), aht.last_hum_pct());
  // Light sensor
  ambientLight.read();
  Serial.printf("Light sensor reading: %d\n", ambientLight.last_ambient_light());
  // SHT40
  #ifdef ADAFRUIT_SHT4x_H
  sensor_return = sht4x.readSht40();
  if (sensor_return == E_SENSOR_SUCCESS) {
    prim_temp_f = sht4x.last_temp_f();
    prim_hum = sht4x.last_hum_pct();
    Serial.printf("SHT40: %0.1f *F  %0.2f rH\n", sht4x.last_temp_f(), sht4x.last_hum_pct());
  } else {
    Serial.printf("ERROR - SHT40 read returned: %d\n", sensor_return);
  }
  #endif
  #ifdef ADAFRUIT_SGP30_H
  sensor_return = sgp30.readSgp30(TEMP_C(prim_temp_f), prim_hum);
  if (sensor_return == E_SENSOR_SUCCESS) {
    Serial.printf("SGP30: %d eCO2 ppm  %d tvoc ppb \n", sgp30.last_eco2(), sgp30.last_tvoc());
    Serial.printf("SGP30: %d ethanol ppm  %d H2 ppm \n", sgp30.last_raw_ethanol(), sgp30.last_raw_h2());
  } else {
    Serial.printf("ERROR - SGP30 read returned: %d\n", sensor_return);
  }
  #endif

  return;
}


void fh_tone(uint8_t pin, float frequency, float duration) {
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
  uint8_t retval;
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("msg:");
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println();
  //Serial.println("-----------------------");
  if (pch = strstr(topic, topic_plants)) {
    retval = get_pepper_mqtt(payload, length);
  } else if (pch = strstr(topic, doors_topic)) {
    unsigned long start = millis();
    retval = get_doors_mqtt(payload, length);
    Serial.printf("Doors sub: %d in %lu ms\n", retval, millis() - start);
  }
  
}


void mqtt_pub_sensors() {
  char mqtt_msg [128];

  // check/reconnect connection to broker
  mqtt_client.mqttReconnect();

  // DPS310
  if ((millis() - dps.last_read_ms()) <= max_mqtt_pub_delay_ms) {
    sprintf(mqtt_msg, "%s,sensor=DPS310,location=%s,room=%s,room_loc=%s temp_f=%f,pressure=%f",
      measurement, location, room, room_loc, dps.last_temp_f(), dps.last_press_hpa());
    mqtt_client.publishTopic(mqtt_msg);
    memset(mqtt_msg, 0, sizeof mqtt_msg);
  }
  // AHT20
  if ((millis() - aht.last_read_ms()) <= max_mqtt_pub_delay_ms) {
    sprintf(mqtt_msg, "%s,sensor=AHT20,location=%s,room=%s,room_loc=%s temp_f=%f,humidity=%f",
      measurement, location, room, room_loc, aht.last_temp_f(), aht.last_hum_pct());
    mqtt_client.publishTopic(mqtt_msg);
    memset(mqtt_msg, 0, sizeof mqtt_msg);
  }
  // Ambient light
  if ((millis() - ambientLight.last_read_ms()) <= max_mqtt_pub_delay_ms) {
    sprintf(mqtt_msg, "%s,sensor=funhouse,location=%s,room=%s,room_loc=%s light=%d",
      measurement, location, room, room_loc, ambientLight.last_ambient_light());
    mqtt_client.publishTopic(mqtt_msg);
    memset(mqtt_msg, 0, sizeof mqtt_msg);
  }
  // SHT40
  #ifdef ADAFRUIT_SHT4x_H
  if ((millis() - sht4x.last_read_ms()) <= max_mqtt_pub_delay_ms) {
    sprintf(mqtt_msg, "%s,sensor=SHT40,location=%s,room=%s,room_loc=%s temp_f=%f,humidity=%f",
      measurement, location, room, room_loc, sht4x.last_temp_f(), sht4x.last_hum_pct());
    mqtt_client.publishTopic(mqtt_msg);
    memset(mqtt_msg, 0, sizeof mqtt_msg);
  }
  #endif
  #ifdef SENSIRIONI2CSCD4X_H
  // SCD40
  if (scd4x.present() && ((millis() - scd4x.last_update_ms()) <= max_mqtt_pub_delay_ms)) {
    sprintf(mqtt_msg, "%s,sensor=SCD40,location=%s,room=%s,room_loc=%s co2=%d,temp_f=%f,humidity=%f",
      measurement, location, room, room_loc, scd4x.last_co2_ppm(), scd4x.last_temp_f(), scd4x.last_hum_pct());
    mqtt_client.publishTopic(mqtt_msg);
    memset(mqtt_msg, 0, sizeof mqtt_msg);
  }
  #endif
  // SGP30
  #ifdef ADAFRUIT_SGP30_H
  if (sgp30.present() & (millis() - sgp30.last_read_ms()) <= max_mqtt_pub_delay_ms) {
    sprintf(mqtt_msg, "%s,sensor=SGP30,location=%s,room=%s,room_loc=%s tvoc=%d,eco2=%d",
      measurement, location, room, room_loc, sgp30.last_tvoc(), sgp30.last_eco2());
    mqtt_client.publishTopic(mqtt_msg);
    memset(mqtt_msg, 0, sizeof mqtt_msg);
    sprintf(mqtt_msg, "%s,sensor=SGP30,location=%s,room=%s,room_loc=%s h2=%d,ethanol=%d",
      measurement, location, room, room_loc, sgp30.last_raw_h2(), sgp30.last_raw_ethanol());
    mqtt_client.publishTopic(mqtt_msg);
    memset(mqtt_msg, 0, sizeof mqtt_msg);
  }
  #endif
  sprintf(mqtt_msg, "wifi,location=%s,room=%s,room_loc=%s,ssid=%s,host=%s rssi=%d",
    location, room, room_loc, fh_wifi.SSID().c_str(), fh_wifi.getHostname(), fh_wifi.RSSI());
  mqtt_client.publish(topic_infra, mqtt_msg);
  memset(mqtt_msg, 0, sizeof mqtt_msg);

  return;
}