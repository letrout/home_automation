#ifndef GARAGE_H
#define GARAGE_H 1


const int side_door_pin = D1;
const int main_door_pin = D2;
const char* location = "Owens";
const char* room = "garage";
const char* side_room_loc = "side";
const char* main_room_loc = "main";
const char* type = "door";

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
