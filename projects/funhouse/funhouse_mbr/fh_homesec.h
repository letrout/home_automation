#pragma once
#ifndef FH_HOMESEC_H
#define FH_HOMESEC_H

#include "fh_mqtt.h"

#define ROOM_LOC_LEN 16

class OwensDoor {
    private:
        bool is_open_;
        unsigned long last_update_;
        unsigned long last_open_;
        char room_[ROOM_LOC_LEN] = {};
        char loc_[ROOM_LOC_LEN] = {};
    public:
        OwensDoor(const char* room, const char* loc);
        bool is_open() { return is_open_; }
        unsigned long last_update() { return last_update_; }
        unsigned long last_open() { return last_open_; }
        const char* room() const { return room_; }
        const char* loc() const { return loc_; }
};

/**
 * @brief Get door sensors from MQTT
 * 
 * @param payload MQTT payload
 * @param length length of payload
 * @return int8_t error code (0 on success)
 */
int8_t get_doors_mqtt(const byte* payload, const int length);

#endif /* FH_HOMESEC_H */