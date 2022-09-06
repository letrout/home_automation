/**
 * @file fh_tft.cpp
 * @author Joel Luth (joel.luth@gmail.com)
 * @brief Classes to manage TFT behavior used on Adafruit Funhouse
 * @version 0.1
 * @date 2022-07-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "fh_tft.h"
#include "fh_sensors.h"
#include "fh_mqtt.h"
#include "fh_time.h"
#include "fh_homesec.h"

// sensors objects
extern FhAmbientLight ambientLight;
extern float prim_temp_f, prim_hum;
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
#ifdef FH_HOMESEC_H
extern std::array<OwensDoor, 5> owensDoors;
#endif

extern uint8_t peppers[];
extern FhNtpClient ntp_client;
extern FhWifi fh_wifi;

// display!
FhTft tft = FhTft(TFT_CS, TFT_DC, TFT_RESET);

// Funhouse ST7789 TFT display
void FhTft::setup(void) {
    init(240, 240);                // Initialize ST7789 screen
    pinMode(TFT_BACKLIGHT, OUTPUT);
    return;
}

uint8_t FhTft::setDisplayMode(byte mode, bool fill) {
    uint8_t retval = 0;
    display_mode_ = mode;
    switch(display_mode_) {
        // TODO: make the sleep mode the default?
        case DISPLAY_MODE_SLEEP :
            digitalWrite(TFT_BACKLIGHT, LOW); // Backlight off
            break;
        case DISPLAY_MODE_ENVIRONMENTAL :
            digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on
            if (fill) { fillScreen(BG_COLOR); }
            setCursor(0, 0);
            setTextSize(3);
            setTextColor(ST77XX_YELLOW);
            setTextWrap(false);
            break;
        case DISPLAY_MODE_ALL_SENSORS :
            digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on
            if (fill) { fillScreen(BG_COLOR); }
            setCursor(0, 0);
            setTextSize(2);
            setTextColor(ST77XX_YELLOW);
            setTextWrap(false);
            break;
        default:
            retval = 1;
    }
    return retval;
}

void FhTft::displaySensors(bool fill) {
  setDisplayMode(DISPLAY_MODE_ALL_SENSORS, fill);

  // DPS310
  setTextColor(ST77XX_YELLOW, BG_COLOR);
  print("DP310: ");
  print(dps.last_temp_f(), 0);
  print(" F ");
  print(dps.last_press_hpa(), 0);
  print(" hPa");
  println("              ");

  // AHT20
  setTextColor(ST77XX_YELLOW, BG_COLOR);
  print("AHT20: ");
  print(aht.last_temp_f(), 0);
  print(" F ");
  print(aht.last_hum_pct(), 0);
  print(" %");
  println("              ");

#ifdef ADAFRUIT_SHT4x_H
  // SHT40
  if (sht4x.present()) {
    setTextColor(ST77XX_YELLOW, BG_COLOR);
    print("SHT40: ");
    print(sht4x.last_temp_f(), 0);
    print(" F ");
    print(sht4x.last_hum_pct(), 0);
    print(" %");
    println("              ");
  }
#endif

#ifdef SENSIRIONI2CSCD4X_H
  // SCD40
  if (scd4x.present()) {
    setTextColor(ST77XX_YELLOW, BG_COLOR);
    print("SCD4x: ");
    print(scd4x.last_co2_ppm(), 0);
    println(" ppm ");
    print("SCD4x: ");
    print(scd4x.last_temp_f(), 0);
    print(" F ");
    print(scd4x.last_hum_pct(), 0);
    print(" %");
    println("              ");
  }
#endif

#ifdef ADAFRUIT_SGP30_H
  // SGP30
  if (sgp30.present()) {
    setTextColor(ST77XX_YELLOW, BG_COLOR);
    print("SGP30: ");
    print("TVOC ");
    print(sgp30.last_tvoc(), 0);
    println(" ppb ");
    print("eCO2 ");
    print(sgp30.last_eco2(), 0);
    println(" ppm");

    setTextColor(ST77XX_YELLOW, BG_COLOR);
    print("H2 ");
    print(sgp30.last_raw_h2(), 0);
    print(" Eth ");
    print(sgp30.last_raw_ethanol());
    println("");
  }
#endif

  // Light sensor
  setTextColor(ST77XX_YELLOW, BG_COLOR);
  print("Light: ");
  setTextColor(ST77XX_WHITE, BG_COLOR);
  print(ambientLight.last_ambient_light());
  println("    ");

#ifdef FH_SUB_PEPPERS
  // Pepper plant soil moisture (from MQTT)
  setTextColor(ST77XX_YELLOW, BG_COLOR);
  print("Pepper:");
  for (int i=0; i < PEPPER_PLANTS; i++) {
    print(" ");
    print(peppers[i]);
  }
  println("");
#endif
  setTextColor(ST77XX_YELLOW, BG_COLOR);
  print("WiFi: ");
  println(fh_wifi.RSSI());

  return;
}

void FhTft::displayEnvironment(bool fill) {
  setDisplayMode(DISPLAY_MODE_ENVIRONMENTAL, fill);
  // Time
  char timeStr[9];
  ntp_client.getFormattedTime(timeStr);
  setTextColor(ST77XX_GREEN, BG_COLOR);
  println(timeStr);
  // Temp and humidity
  setTextColor(ST77XX_YELLOW, BG_COLOR);
  print(prim_temp_f, 0);
  setTextColor(ST77XX_GREEN, BG_COLOR);
  print(" F  ");
  setTextColor(ST77XX_YELLOW, BG_COLOR);
  print(prim_hum, 0);
  setTextColor(ST77XX_GREEN, BG_COLOR);
  println(" %");
  // Pressure
  setTextColor(ST77XX_YELLOW, BG_COLOR);
  //print(dps.last_press_hpa(), 0);
  this->printf("%.2f", dps.inHgAdjusted(), 0);
  setTextColor(ST77XX_GREEN, BG_COLOR);
  //println(" hPa");
  println(" inHg");
#ifdef SENSIRIONI2CSCD4X_H
  // CO2
  if (scd4x.present()) {
    setTextColor(ST77XX_YELLOW, BG_COLOR);
    print(scd4x.last_co2_ppm());
    setTextColor(ST77XX_GREEN, BG_COLOR);
    println(" ppm CO2");
  }
#endif
#ifdef ADAFRUIT_SGP30_H
//#ifndef SENSIRIONI2CSCD4X_H
  if (sgp30.present()) {
    setTextColor(ST77XX_YELLOW, BG_COLOR);
    print(sgp30.last_eco2());
    setTextColor(ST77XX_GREEN, BG_COLOR);
    println(" ppm eCO2");
  //#endif /* SENSIRIONI2CSCD4X_H */
    setTextColor(ST77XX_YELLOW, BG_COLOR);
    print(sgp30.last_tvoc());
    setTextColor(ST77XX_GREEN, BG_COLOR);
    println(" ppb TVOC");
  }
#endif /* ADAFRUIT_SGP30_H */
}

void FhTft::displayDoors(bool fill) {
#ifdef FH_HOMESEC_H
  setDisplayMode(DISPLAY_MODE_ENVIRONMENTAL, fill);
  for (auto &door: owensDoors) {
    //Serial.printf("get state door %d...\n", i);
    door.getCurrentState();
  }
  setTextColor(ST77XX_YELLOW, BG_COLOR);
  print("Back: ");
  if (owensDoors[2].is_open()) {
    setTextColor(ST77XX_RED, BG_COLOR);
  } else {
    setTextColor(ST77XX_GREEN, BG_COLOR);
  }
  println(owensDoors[2].is_open());
  setTextColor(ST77XX_YELLOW, BG_COLOR);
  print("Deck: ");
  if (owensDoors[3].is_open()) {
    setTextColor(ST77XX_RED, BG_COLOR);
  } else {
    setTextColor(ST77XX_GREEN, BG_COLOR);
  }
  println(owensDoors[3].is_open());
  setTextColor(ST77XX_YELLOW, BG_COLOR);
  print("Front: ");
  if (owensDoors[4].is_open()) {
    setTextColor(ST77XX_RED, BG_COLOR);
  } else {
    setTextColor(ST77XX_GREEN, BG_COLOR);
  }
  println(owensDoors[4].is_open());
  setTextColor(ST77XX_YELLOW, BG_COLOR);
  print("GMain: ");
  if (owensDoors[0].is_open()) {
    setTextColor(ST77XX_RED, BG_COLOR);
  } else {
    setTextColor(ST77XX_GREEN, BG_COLOR);
  }
  println(owensDoors[0].is_open());
  setTextColor(ST77XX_YELLOW, BG_COLOR);
  print("GSide: ");
  if (owensDoors[1].is_open()) {
    setTextColor(ST77XX_RED, BG_COLOR);
  } else {
    setTextColor(ST77XX_GREEN, BG_COLOR);
  }
  println(owensDoors[1].is_open());
#endif
}