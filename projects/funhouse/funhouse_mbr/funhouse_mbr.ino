// Adafruit FunHouse in MBR

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
#include "funhouse_mbr.h"
#include "secrets.h"

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

// Sensors
sensors_event_t dps_temp, dps_pressure;
sensors_event_t aht_humidity, aht_temp;
sensors_event_t sht_humidity, sht_temp;
uint16_t scd4x_co2, ambient_light;
uint16_t sgp_tvoc, sgp_eco2, sgp_raw_h2, sgp_raw_ethanol;
float scd4x_temp, scd4x_hum;
float prim_temp_c, prim_hum;  // primary temp and humidity measurements

// timers
const unsigned long mqtt_ms = 60000;  // publish to mqtt every x ms
unsigned long mqtt_last_ms = 0;
const unsigned long sensor_ms = 1000;  // read sensors every x ms
unsigned long sensor_last_ms = 0;
const unsigned long scd4x_ms = 5000; // read SCD4x sensors every x ms
unsigned long scd4x_last_ms = 0;

uint8_t LED_dutycycle = 0;
uint16_t firstPixelHue = 0;
const uint8_t tft_line_step = 20; // number of pixels in each tft line of text 
bool has_sht4x = false;
bool has_scd4x = false;
bool has_sgp30 = false;
const char* measurement = "environment";

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

  tft.fillScreen(BG_COLOR);
} // setup()


void loop() {
  uint16_t scd4x_error;
  char mqtt_msg [128];
  uint8_t cursor_y = 0;

  has_scd4x ? delay(5000) : delay(1000);

  // timers
  unsigned long now = millis();
  bool sensors_update = false;
  bool scd4x_update = false;
  bool mqtt_pubnow = false;

  // check timers
  if ((now - sensor_last_ms) > sensor_ms) {
    sensors_update = true;
    sensor_last_ms = now;
    read_sensors();
  } else {
    sensors_update = false;
  }
  if ((now - scd4x_last_ms) > scd4x_ms) {
    scd4x_update = true;
    scd4x_last_ms = now;
  } else {
    scd4x_update = false;
  }

  cursor_y = display_sensors(cursor_y);

  // MQTT publish interval expired?
  if ((millis() - mqtt_last_ms) > mqtt_ms) {
    mqtt_pubnow = true;
    mqtt_last_ms = millis();
  } else {
    client.loop();
  }
  
  // DPS310
  Serial.printf("DPS310: %0.1f *F  %0.2f hPa\n", TEMP_F(dps_temp.temperature), dps_pressure.pressure);
  if (mqtt_pubnow) {
    sprintf(mqtt_msg, "%s,sensor=DPS310 temp_f=%f,pressure=%f", measurement, TEMP_F(dps_temp.temperature), dps_pressure.pressure);
    client.publish(topic, mqtt_msg);
    memset(mqtt_msg, 0, sizeof mqtt_msg);
  }

  // AHT20
  Serial.printf("AHT20: %0.1f *F  %0.2f rH\n", TEMP_F(aht_temp.temperature), aht_humidity.relative_humidity);
  if (mqtt_pubnow) {
    sprintf(mqtt_msg, "%s,sensor=AHT20 temp_f=%f,humidity=%f", measurement, TEMP_F(aht_temp.temperature), aht_humidity.relative_humidity);
    client.publish(topic, mqtt_msg);
    memset(mqtt_msg, 0, sizeof mqtt_msg);
  }

  // SHT40
  if (has_sht4x) {
    Serial.printf("SHT40: %0.1f *F  %0.2f rH\n", TEMP_F(sht_temp.temperature), sht_humidity.relative_humidity);
    if (mqtt_pubnow) {
      sprintf(mqtt_msg, "%s,sensor=SHT40 temp_f=%f,humidity=%f", measurement, TEMP_F(sht_temp.temperature), sht_humidity.relative_humidity);
      client.publish(topic, mqtt_msg);
      memset(mqtt_msg, 0, sizeof mqtt_msg);
    }
  }

   // SCD40
  if (has_scd4x && scd4x_update) {
    // TODO: setAmbientPressure() with value from DPS310?
    scd4x_error = read_scd4x();
    if (scd4x_error) {
      // tft.print("error ");
      // tft.print(scd4x_error, 0);
      Serial.printf("SCD4x error %s\n", scd4x_error);
    } else if (scd4x_co2 == 0){
      // tft.print("error reading CO2");
      Serial.printf("SCD4x error: CO2 reading 0\n");
    } else {
      Serial.printf("SCD4x: %d ppm %0.1f *C  %0.2f rH\n", scd4x_co2, scd4x_temp, scd4x_hum);
      if (mqtt_pubnow) {
        sprintf(mqtt_msg, "%s,sensor=SCD40 co2=%d,temp_f=%f,humidity=%f", measurement, scd4x_co2, TEMP_F(scd4x_temp), scd4x_hum);
        client.publish(topic, mqtt_msg);
        memset(mqtt_msg, 0, sizeof mqtt_msg);
      }
    }
  }

  if (has_sgp30) {
    if (mqtt_pubnow) {
      sprintf(mqtt_msg, "%s,sensor=SGP30 tvoc=%d,eco2=%d", measurement, sgp_tvoc, sgp_eco2);
      client.publish(topic, mqtt_msg);
      memset(mqtt_msg, 0, sizeof mqtt_msg);
      sprintf(mqtt_msg, "%s,sensor=SGP30 h2=%d,ethanol=%d", measurement, sgp_raw_h2, sgp_raw_ethanol);
      client.publish(topic, mqtt_msg);
      memset(mqtt_msg, 0, sizeof mqtt_msg);
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

  Serial.printf("Light sensor reading: %d\n", ambient_light);
  if (mqtt_pubnow) {
    sprintf(mqtt_msg, "%s,sensor=funhouse light=%d", measurement, ambient_light);
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
  for (int i=0; i<pixels.numPixels(); i++) { // For each pixel in strip...
      int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
  }
  pixels.show(); // Update strip with new contents
  firstPixelHue += 256;
} // loop()


/**
 * @brief Read the sensors
 * store results in global variables
 */
void read_sensors() {
  // DPS310
  dps.getEvents(&dps_temp, &dps_pressure);
  prim_temp_c = dps_temp.temperature;
  // AHT20
  aht.getEvent(&aht_humidity, &aht_temp);
  prim_temp_c = aht_temp.temperature;
  prim_hum = aht_humidity.relative_humidity;
  // Light sensor
  ambient_light = analogRead(A3);
  // SHT40
  if (has_sht4x) {
    sht4x.getEvent(&sht_humidity, &sht_temp);
    prim_temp_c = sht_temp.temperature;
    prim_hum = sht_humidity.relative_humidity;
  }
  // SGP30
  // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
  //float temperature = 22.1; // [°C]
  //float humidity = 45.2; // [%RH]
  if (has_sgp30) {
    sgp30.setHumidity(getAbsoluteHumidity(prim_temp_c, prim_hum));
    if (! sgp30.IAQmeasure()) {
      Serial.println("SGP30 Measurement failed");
    } else {
      sgp_tvoc = sgp30.TVOC;
      sgp_eco2 = sgp30.eCO2;
    }
    if (! sgp30.IAQmeasureRaw()) {
      Serial.println("SGP30 Raw Measurement failed");
    } else {
      sgp_raw_h2 = sgp30.rawH2;
      sgp_raw_ethanol = sgp30.rawEthanol;
    }
  }
  return;
}


/**
 * @brief Read the SCD40 sensor
 * store the results in global variables
 * 
 * @return uint16_t error code returned from sensor readMeasurement()
 */
uint16_t read_scd4x() {
  uint16_t error = 0;
  if (has_scd4x) {
    error = scd4x.readMeasurement(scd4x_co2, scd4x_temp, scd4x_hum);
  }
  if (error) {
    // Let the caller handler error?
    // Serial.printf("SCD4x error %s\n", error);
  } else if (scd4x_co2 == 0) {
    // Let the caller handle co2==0?
    // Serial.printf("SCD4x error: CO2 reading 0\n");
  } else if (! has_sht4x) {
    prim_temp_c = scd4x_temp;
    prim_hum = scd4x_hum;
  }
  return error;
}


/**
 * @brief output a tone to the speaker
 * 
 * @param pin 
 * @param frequency 
 * @param duration 
 */
void tone(uint8_t pin, float frequency, float duration) {
  ledcSetup(1, frequency, 8);
  ledcAttachPin(pin, 1);
  ledcWrite(1, 128);
  delay(duration);
  ledcWrite(1, 0);
}


/**
 * @brief Set the up scd4x object
 * 
 * @param scd4x SCD40 object
 * @return uint16_t error value
 */
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


/**
 * @brief serial print an int in hexadecimal
 * 
 * @param value the integer value to be printed
 */
void printUint16Hex(uint16_t value) {
  Serial.print(value < 4096 ? "0" : "");
  Serial.print(value < 256 ? "0" : "");
  Serial.print(value < 16 ? "0" : "");
  Serial.print(value, HEX);
}


/**
 * @brief serial print a serial number
 * 
 * @param serial0 
 * @param serial1 
 * @param serial2 
 */
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


/**
 * @brief MQTT subscribe callback function
 * 
 * @param topic the topic of the message
 * @param payload the message payload
 * @param length length of the message payload
 */
void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}


uint8_t display_sensors(const uint8_t cursor_y_start) {
  uint8_t cursor_y = cursor_y_start;
  const uint8_t tft_line_step = 20;

  // DPS310
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("DP310: ");
  tft.print(TEMP_F(dps_temp.temperature), 0);
  tft.print(" F ");
  tft.print(dps_pressure.pressure, 0);
  tft.print(" hPa");
  tft.println("              ");

  // AHT20
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("AHT20: ");
  tft.print(TEMP_F(aht_temp.temperature), 0);
  tft.print(" F ");
  tft.print(aht_humidity.relative_humidity, 0);
  tft.print(" %");
  tft.println("              ");

  // SHT40
  if (has_sht4x) {
    tft.setCursor(0, cursor_y);
    cursor_y += tft_line_step;
    tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
    tft.print("SHT40: ");
    tft.print(TEMP_F(sht_temp.temperature), 0);
    tft.print(" F ");
    tft.print(sht_humidity.relative_humidity, 0);
    tft.print(" %");
    tft.println("              ");
  }

  // SCD40
  if (has_scd4x) {
    tft.setCursor(0, cursor_y);
    cursor_y += tft_line_step;
    tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
    tft.print("SCD4x: ");
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
  }

  // SGP30
  if (has_sgp30) {
    tft.setCursor(0, cursor_y);
    cursor_y += tft_line_step;
    tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
    tft.print("SGP30: ");
    // If you have a temperature / humidity sensor, you can set the absolute humidity to enable the humditiy compensation for the air quality signals
    //float temperature = 22.1; // [°C]
    //float humidity = 45.2; // [%RH]
    tft.print("TVOC ");
    tft.print(sgp_tvoc, 0);
    tft.print(" ppb ");
    tft.setCursor(0, cursor_y);
    cursor_y += tft_line_step;
    tft.print("eCO2 ");
    tft.print(sgp_eco2, 0);
    tft.print(" ppm");

    tft.setCursor(0, cursor_y);
    cursor_y += tft_line_step;
    tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
    tft.print("H2 ");
    tft.print(sgp_raw_h2, 0);
    tft.print(" Eth ");
    tft.print(sgp_raw_ethanol, 0);
  }

  // Light sensor
  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("Light: ");
  tft.setTextColor(ST77XX_WHITE, BG_COLOR);
  tft.print(ambient_light);
  tft.println("    ");

  return cursor_y;
}
