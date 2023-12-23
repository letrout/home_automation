#ifndef SENSOR_DEVICE_H
#define SENSOR_DEVICE_H 1
#define KITCHEN 1
#define AMBIENT_LIGHT 1

#define DEBUG 1


const int door_pin = D7;
const char* location = "Owens";
const char* room = "kitchen";
const char* room_loc = "deck";
const char* type = "door";

#define PIR 1
#ifdef PIR
const int pir_pin = D3;
int pir_state = 0;
#endif

#endif // SENSOR_DEVICE_H
