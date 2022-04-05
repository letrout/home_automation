// Adafruit FunHouse DOTSTAR LED test

#include <Adafruit_DotStar.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789

#define NUM_DOTSTAR 5
#define BG_COLOR ST77XX_BLACK

// display!
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RESET);
// LEDs!
Adafruit_DotStar pixels(NUM_DOTSTAR, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLOCK, DOTSTAR_BRG);
uint8_t leds[NUM_DOTSTAR] = {100, 75, 50, 25, 0};
const uint8_t tft_line_step = 20;
uint16_t ambient_light;

void setup() {
  uint8_t cursor_y = 0;

  //while (!Serial);
  Serial.begin(115200);
  delay(100);
  
  pixels.begin(); // Initialize pins for output
  pixels.show();  // Turn all LEDs off ASAP
  pixels.setBrightness(20);

  tft.init(240, 240);                // Initialize ST7789 screen
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on

  tft.fillScreen(BG_COLOR);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextWrap(false);
}


void loop() {
  uint8_t cursor_y = 0;

  // Light sensor
  ambient_light = analogRead(A3);

  // Set dotstars to pepper plant moisture (from MQTT)
  uint16_t led_hues[NUM_DOTSTAR];
  for (int i=0; i < NUM_DOTSTAR; i++) {
    led_hues[i] = map(leds[i], 0, 100, 0, 44000); // 0=red, 100=blue
    Serial.print("led: ");
    Serial.print(leds[i]);
    Serial.print(", hue: ");
    Serial.println(led_hues[i]);
    tft.setCursor(0, cursor_y);
    cursor_y += tft_line_step;
    tft.print("val: ");
    tft.print(leds[i]);
    tft.print(" hue: ");
    tft.println(led_hues[i]);
    pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(led_hues[i])));
  }
  /*
  pixels.setPixelColor(0, pixels.gamma32(pixels.ColorHSV(led_hues[0])));
  pixels.setPixelColor(1, pixels.gamma32(pixels.ColorHSV(led_hues[1])));
  pixels.setPixelColor(2, pixels.gamma32(pixels.ColorHSV(led_hues[2])));
  pixels.setPixelColor(3, pixels.gamma32(pixels.ColorHSV(led_hues[3])));
  pixels.setPixelColor(4, pixels.gamma32(pixels.ColorHSV(led_hues[4])));
  */

  pixels.show(); // Update strip with new contents

  delay(1000);
}