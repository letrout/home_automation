#ifndef DOOR_MONITOR_H
#define DOOR_MONITOR_H 1


/**
 * @brief print the current time to serial
 * 
 */
void print_time();


/**
 * @brief MQTT subscribe callback function
 * 
 * @param topic the topic of the message
 * @param payload the message payload
 * @param length length of the message payload
 */
void callback(char *topic, byte *payload, unsigned int length);

#endif
