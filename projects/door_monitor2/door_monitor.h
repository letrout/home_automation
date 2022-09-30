#ifndef DOOR_MONITOR_H
#define DOOR_MONITOR_H 1


/**
 * @brief MQTT publish door state, in Influxdb line protocol
 * 
 * @param state current door state
 * @return boolean 
 */
boolean mqtt_pub_door(int state);

/**
 * @brief print the current time to serial
 * 
 */
void print_time(void);


/**
 * @brief MQTT subscribe callback function
 * 
 * @param topic the topic of the message
 * @param payload the message payload
 * @param length length of the message payload
 */
void callback(char *topic, byte *payload, unsigned int length);


/**
 * @brief check if connected to mqtt broker, reconnect loop if necessary
 * This is blocking. Right now this app only exists to publish to MQTT,
 * but if that changes this will need to change to something non-blocking
 * 
 */
void mqtt_reconnect(void);

#endif
