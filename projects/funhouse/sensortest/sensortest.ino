// For testing sensors with the Adafruit FunHouse

#include <Adafruit_DotStar.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Adafruit_DPS310.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_SHT4x.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>

#define NUM_DOTSTAR 5
#define BG_COLOR ST77XX_BLACK

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

uint8_t LED_dutycycle = 0;
uint16_t firstPixelHue = 0;
const uint8_t tft_line_step = 20; // number of pixels in each tft line of text 
bool has_sht4x = false;
bool has_scd4x = false;

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

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SPEAKER, OUTPUT);

  ledcSetup(0, 2000, 8);
  ledcAttachPin(LED_BUILTIN, 0);

  ledcSetup(1, 2000, 8);
  ledcAttachPin(SPEAKER, 1);
  ledcWrite(1, 0);
}


void loop() {
  uint8_t cursor_y = 0;

  delay(5000);
  tft.fillScreen(BG_COLOR);


  /********************* sensors    */
  sensors_event_t humidity, temp, pressure;
  
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

  tft.setCursor(0, cursor_y);
  cursor_y += tft_line_step;
  tft.setTextColor(ST77XX_YELLOW, BG_COLOR);
  tft.print("Light: ");
  analogread = analogRead(A3);
  tft.setTextColor(ST77XX_WHITE, BG_COLOR);
  tft.print(analogread);
  tft.println("    ");
  Serial.printf("Light sensor reading: %d\n", analogread);
  
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
}


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
