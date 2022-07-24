#ifndef FH_MBR_H
#define FH_MBR_H

#include "fh_sensors.h"

// sensors objects
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

/**
 * @brief Read the sensors
 * store results in global variables
 */
void read_sensors(void);


/**
 * @brief Read the SCD40 sensor
 * store the results in global variables
 * 
 * @return uint16_t error code returned from sensor readMeasurement()
 */
uint16_t read_scd4x(void);


/**
 * @brief output a tone to the speaker
 * 
 * @param pin 
 * @param frequency 
 * @param duration 
 */
void tone(uint8_t pin, float frequency, float duration);


/**
 * @brief Set the up scd4x object
 * 
 * @param scd4x SCD40 object
 * @return uint16_t error value
 */
uint16_t setup_scd4x(SensirionI2CScd4x& scd4x);


/**
 * @brief serial print an int in hexadecimal
 * 
 * @param value the integer value to be printed
 */
void printUint16Hex(uint16_t value);


/**
 * @brief serial print a serial number
 * 
 * @param serial0 
 * @param serial1 
 * @param serial2 
 */
void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2);


/**
 * @brief MQTT subscribe callback function
 * 
 * @param topic the topic of the message
 * @param payload the message payload
 * @param length length of the message payload
 */
void callback(char *topic, byte *payload, unsigned int length);


/**
 * @brief display sensor values on the TFT
 * 
 * @param uint8_t starting cursor position
 * @return uint8_t the final cursor value
 */
uint8_t display_sensors(const uint8_t cursor_y_start = 0);


/**
 * @brief publish sensor readings to MQTT broker
 * 
 */
void mqtt_pub_sensors(void);


/**
 * @brief Get the pepper plant moisture level from MQTT
 * 
 * @param payload MQTT payload
 * @param length length of payload
 * @return int8_t error code (0 on success)
 */
int8_t get_pepper_mqtt(const byte* payload, const int length);


/**
 * @brief check if connected to mqtt broker, reconnect loop if necessary
 * This is blocking. Right now this app only exists to publish to MQTT,
 * but if that changes this will need to change to something non-blocking
 * 
 */
void mqtt_reconnect(void);

#endif // FH_MBR_H