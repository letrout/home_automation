// For testing sensors with the Adafruit FunHouse

#include <Adafruit_DotStar.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Adafruit_DPS310.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_SGP30.h>
#include <Adafruit_SHT4x.h>
#include <PubSubClient.h>
#include <SensirionI2CScd4x.h>
#include <WiFi.h>
#include <Wire.h>
#include "secrets.h"

#define ARRAY_LENGTH(array) (sizeof(array)/sizeof((array)[0]))
#define NUM_DOTSTAR 5
#define BG_COLOR ST77XX_BLACK
#define ALT_M 285 // altitude in meters, for SCD-4x calibration

#define TEMP_F(c) (c * 9 / 5) + 32

// display!
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RESET);
// LEDs!
Adafruit_DotStar pixels(NUM_DOTSTAR, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLOCK, DOTSTAR_BRG);
// sensors!
Adafruit_DPS310 dps;
Adafruit_AHTX0 aht;
Adafruit_SHT4x sht4x = Adafruit_SHT4x();
SensirionI2CScd4x scd4x;
Adafruit_SGP30 sgp30;

uint8_t LED_dutycycle = 0;
uint16_t firstPixelHue = 0;
const uint8_t tft_line_step = 20; // number of pixels in each tft line of text 
bool has_sht4x = false;
bool has_scd4x = false;
bool has_sgp30 = false;
const unsigned long mqtt_ms = 60000;
unsigned long mqtt_last_ms=0;
boolean mqtt_pubnow = false;
const char* measurement = "environment";
char client_id[16] = "fh-"; // will be the MQTT client ID, after MAC appended

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
  pixels.setBrightness(20);

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

  
  if (! dps.begin_I2C()) {  
    tft.setTextColor(ST77XX_RED);
    tft.println("FAIL!");
    while (1) delay(100);
  }
  tft.setTextColor(ST77XX_GREEN);
  tft.println("OK!");
  dps.configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
  dps.configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);

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
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("SHT4x? ");
  retries = 5, i = 0;
  while (i < retries) {
    if (! sht4x.begin()) {  
      tft.setTextColor(ST77XX_RED);
      tft.println("FAIL!");
      delay(100);
      i++;
    } else {
      has_sht4x = true;
      tft.setTextColor(ST77XX_GREEN);
      tft.println("OK!");
      break;
    }
  }

  // check SCD-4X!
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("SCD-4X? ");
  retries = 5, i = 0;
  Wire.begin();
  while (i < retries) {
    if (setup_scd4x(scd4x)) {  
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

  // check SGP30!
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("SGP30? ");
  retries = 5, i = 0;
  while (i < retries) {
    if (! sgp30.begin()) {  
      tft.setTextColor(ST77XX_RED);
      tft.println("FAIL!");
      delay(100);
      i++;
    } else {
      has_sgp30 = true;
      tft.setTextColor(ST77XX_GREEN);
      tft.println("OK!");
      Serial.print("Found SGP30 serial #");
      Serial.print(sgp30.serialnumber[0], HEX);
      Serial.print(sgp30.serialnumber[1], HEX);
      Serial.println(sgp30.serialnumber[2], HEX);
      break;
    }
  }

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

  // Connect to MQTT
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
 client.subscribe("influx/Owens/sensors/mbr/door");

  tft.fillScreen(BG_COLOR);
} // setup()


void loop() {
  uint8_t cursor_y = 0;
  /********************* sensors    */
  sensors_event_t humidity, temp, pressure;
  uint16_t scd4x_co2, error, light;
  float scd4x_temp, scd4x_hum;
  float prim_temp_c, prim_hum;  // primary temp and humidity measurements
  char mqtt_msg [128];

  has_scd4x ? delay(5000) : delay(1000);
  // MQTT publish interval expired?
  // Serial.printf("current: %lu, last: %lu, interval: %lu\n", millis(), mqtt_last_ms, mqtt_ms);
  if ((millis() - mqtt_last_ms) > mqtt_ms) {
    if (!client.connected()) {
      Serial.println("Reconnecting MQTT...");
      if (client.connect(client_id, mqtt_username, mqtt_password)) {
        Serial.println("MQTT reconnected");
        mqtt_pubnow = true;
        mqtt_last_ms = millis();
      } else {
        Serial.println("Failed to reconnect MQTT!");
      }
    } else {
      mqtt_pubnow = true;
      mqtt_last_ms = millis();
    }
  } else {
    mqtt_pubnow = false;
    client.loop();
  }
  
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  dps.getEvents(&temp, &pressure);
  
  tft.print("DP310: ");
  tft.print(TEMP_F(temp.temperature), 0);
  tft.print(" F ");
  tft.print(pressure.pressure, 0);
  tft.print(" hPa");
  tft.println("              ");
  Serial.printf("DPS310: %0.1f *F  %0.2f hPa\n", TEMP_F(temp.temperature), pressure.pressure);
  if (mqtt_pubnow) {
    sprintf(mqtt_msg, "%s,sensor=DPS310 temp_f=%f,pressure=%f", measurement, TEMP_F(temp.temperature), pressure.pressure);
    client.publish(topic, mqtt_msg);
    memset(mqtt_msg, 0, sizeof mqtt_msg);
  }
  prim_temp_c = temp.temperature;

  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  aht.getEvent(&humidity, &temp);

  tft.print("AHT20: ");
  tft.print(TEMP_F(temp.temperature), 0);
  tft.print(" F ");
  tft.print(humidity.relative_humidity, 0);
  tft.print(" %");
  tft.println("              ");
  Serial.printf("AHT20: %0.1f *F  %0.2f rH\n", TEMP_F(temp.temperature), humidity.relative_humidity);
  if (mqtt_pubnow) {
    sprintf(mqtt_msg, "%s,sensor=AHT20 temp_f=%f,humidity=%f", measurement, TEMP_F(temp.temperature), humidity.relative_humidity);
    client.publish(topic, mqtt_msg);
    memset(mqtt_msg, 0, sizeof mqtt_msg);
  }
  prim_temp_c = temp.temperature;
  prim_hum = humidity.relative_humidity;

  if (has_sht4x) {
    tft.setCursor(0, cursor_y);
    cursor_y += tft_line_step;
    tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
    sht4x.getEvent(&humidity, &temp);
    tft.print("SHT40: ");
    tft.print(TEMP_F(temp.temperature), 0);
    tft.print(" F ");
    tft.print(humidity.relative_humidity, 0);
    tft.print(" %");
    tft.println("              ");
    Serial.printf("SHT40: %0.1f *F  %0.2f rH\n", TEMP_F(temp.temperature), humidity.relative_humidity);
    if (mqtt_pubnow) {
      sprintf(mqtt_msg, "%s,sensor=SHT40 temp_f=%f,humidity=%f", measurement, TEMP_F(temp.temperature), humidity.relative_humidity);
      client.publish(topic, mqtt_msg);
      memset(mqtt_msg, 0, sizeof mqtt_msg);
    }
    prim_temp_c = temp.temperature;
    prim_hum = humidity.relative_humidity;
  }

  if (has_scd4x) {
    tft.setCursor(0, cursor_y);
    cursor_y += tft_line_step;
    tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
    // TODO: setAmbientPressure() with value from DPS310?
    error = scd4x.readMeasurement(scd4x_co2, scd4x_temp, scd4x_hum);
    tft.print("SCD4x: ");
    if (error) {
      tft.print("error ");
      tft.print(error, 0);
      Serial.printf("SCD4x error %s\n", error);
    } else if (scd4x_co2 == 0){
      tft.print("error reading CO2");
      Serial.printf("SCD4x error: CO2 reading 0\n");
    } else {
      tft.print(scd4x_co2, 0);
      tft.print(" ppm ");
      tft.setCursor(0, cursor_y);
      cursor_y += tft_line_step;
      tft.print("SCD4x: ");
      tft.print(TEMP_F(scd4x_temp), 0);
      tft.print(" F ");
      tft.print(scd4x_hum, 0);
      tft.print(" %");
      tft.println("              ");
      Serial.printf("SCD4x: %d ppm %0.1f *C  %0.2f rH\n", scd4x_co2, scd4x_temp, scd4x_hum);
      if (mqtt_pubnow) {
        sprintf(mqtt_msg, "%s,sensor=SCD40 co2=%d,temp_f=%f,humidity=%f", measurement, scd4x_co2, TEMP_F(scd4x_temp), scd4x_hum);
        client.publish(topic, mqtt_msg);
        memset(mqtt_msg, 0, sizeof mqtt_msg);
      }
      if (! has_sht4x) {
        prim_temp_c = scd4x_temp;
        prim_hum = scd4x_hum;
      }
    }
  }

  if (has_sgp30) {
    tft.setCursor(0, cursor_y);
    cursor_y += tft_line_step;
    tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
    tft.print("SGP30: ");
    // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
    //float temperature = 22.1; // [°C]
    //float humidity = 45.2; // [%RH]
    sgp30.setHumidity(getAbsoluteHumidity(prim_temp_c, prim_hum));
    if (! sgp30.IAQmeasure()) {
      Serial.println("SGP30 Measurement failed");
    } else {
      tft.print("TVOC ");
      tft.print(sgp30.TVOC, 0);
      tft.print(" ppb ");
      tft.setCursor(0, cursor_y);
      cursor_y += tft_line_step;
      tft.print("eCO2 ");
      tft.print(sgp30.eCO2, 0);
      tft.print(" ppm");
      if (mqtt_pubnow) {
        sprintf(mqtt_msg, "%s,sensor=SGP30 tvoc=%d,eco2=%d", measurement, sgp30.TVOC, sgp30.eCO2);
        client.publish(topic, mqtt_msg);
        memset(mqtt_msg, 0, sizeof mqtt_msg);
      }
    }
    if (! sgp30.IAQmeasureRaw()) {
      Serial.println("SGP30 Raw Measurement failed");
    } else {
      tft.setCursor(0, cursor_y);
      cursor_y += tft_line_step;
      tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
      tft.print("H2 ");
      tft.print(sgp30.rawH2, 0);
      tft.print(" Eth ");
      tft.print(sgp30.rawEthanol, 0);
      if (mqtt_pubnow) {
        sprintf(mqtt_msg, "%s,sensor=SGP30 h2=%d,ethanol=%d", measurement, sgp30.rawH2, sgp30.rawEthanol);
        client.publish(topic, mqtt_msg);
        memset(mqtt_msg, 0, sizeof mqtt_msg);
      }
    }
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

  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("Light: ");
  light = analogRead(A3);
  tft.setTextColor(ST77XX_WHITE, BG_COLOR);
  tft.print(light);
  tft.println("    ");
  Serial.printf("Light sensor reading: %d\n", light);
  if (mqtt_pubnow) {
    sprintf(mqtt_msg, "%s,sensor=funhouse light=%d", measurement, light);
    client.publish(topic, mqtt_msg);
    memset(mqtt_msg, 0, sizeof mqtt_msg);
  }
  
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
  uint8_t pixel_bright;
  // dim dotstars as ambient light decreases
  pixel_bright = map(light, 0, 8192, 0, 255);
  for (int i=0; i<pixels.numPixels(); i++) { // For each pixel in strip...
      int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
  }
  pixels.setBrightness(pixel_bright);
  pixels.show(); // Update strip with new contents
  Serial.print("pixel brightness: ");
  Serial.println(pixel_bright);

  firstPixelHue += 256;
} // loop()


void tone(uint8_t pin, float frequency, float duration) {
  ledcSetup(1, frequency, 8);
  ledcAttachPin(pin, 1);
  ledcWrite(1, 128);
  delay(duration);
  ledcWrite(1, 0);
}


uint16_t setup_scd4x(SensirionI2CScd4x& scd4x) {
  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire);
  // stop potentially previously started measurement
  error = scd4x.stopPeriodicMeasurement();
  if (error) {
    Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  uint16_t serial0;
  uint16_t serial1;
  uint16_t serial2;
  error = scd4x.getSerialNumber(serial0, serial1, serial2);
  if (error) {
    Serial.print("Error trying to execute getSerialNumber(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else {
    printSerialNumber(serial0, serial1, serial2);
  }
  // Set altitude
  error = scd4x.setSensorAltitude(ALT_M);
  if (error) {
    Serial.print("Error trying to execute setSensorAltitude(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  // Start Measurement
  error = scd4x.startPeriodicMeasurement();
  if (error) {
    Serial.print("Error trying to execute startPeriodicMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  return error;
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


/* return absolute humidity [mg/m^3] with approximation formula
* @param temperature [°C]
* @param humidity [%RH]
*/
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
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
  float value;
  int ret;
  ret = get_mqtt_val("temp_f", payload, length, &value);
  if (ret == 0) {
    Serial.print("value: ");
    Serial.println(value);
  }
}


// Is there a better way (regex)?
int get_mqtt_val(const char* field, const byte* payload, int length, float* value) {
  int ret = -1;
  char msg[length];
  char *pch;
  //memccpy(msg, payload, sizeof(payload), sizeof(char));
  for (int i = 0; i < length; i++) {
    msg[i] = (char)payload[i];
    Serial.print(msg[i]);
  }
  Serial.println();
  pch = strstr(msg, field); // payload starting at field name
  if (pch != NULL) {
    pch = strtok(pch, "=, "); // split result on delimiters
  }
  if (pch != NULL) {
    pch = strtok(NULL, "=, ");  // get the second token after split
  }
  if ((pch == NULL) || (pch[0] =='\0')) {
    ret = 1;
  } else {
    *value = atof(pch);
    ret = 0;
  }
  /*
  Serial.println(pch);
  if (pch != NULL) {
    // search for field terminator (',' or ' ')
    // skip index zero, which should be '='
    for (int i = 1; i < strlen(pch); i++) {
      if ((pch[i] == ',') || (pch[i] == ' ' )) {
        break;
      }
      value[i] = pch[i];
      ret = 0;
    }
  }
  */
  return ret;
}
