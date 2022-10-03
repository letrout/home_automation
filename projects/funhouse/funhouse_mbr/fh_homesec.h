#pragma once
#ifndef FH_HOMESEC_H
#define FH_HOMESEC_H

#include <map>
#include "fh_mqtt.h"

#define ROOM_LOC_LEN 16

class OwensDoor {
    private:
        bool is_open_ = false;
        unsigned long last_update_ms_;
        time_t last_update_epoch_s_;
        unsigned long last_open_ms_;
        char room_[ROOM_LOC_LEN] = {};
        char loc_[ROOM_LOC_LEN] = {};
    public:
        OwensDoor(const char* room, const char* loc);
        bool is_open() { return is_open_; }
        unsigned long last_update_ms() { return last_update_ms_; }
        unsigned long last_open_ms() { return last_open_ms_; }
        const char* room() const { return room_; }
        const char* loc() const { return loc_; }
        static uint8_t make_key(const char* room, const char* loc, char* key);
        /**
         * @brief Get the time since last open state, in seconds
         * @param uint32_t *seconds - pointer to uint32_t to store result
         * 
         * @return int8_t return code (0 on success)
         */
        uint8_t secSinceOpen(uint32_t *seconds);
        /**
         * @brief GUpdate the object with current door state
         * 
         * @return uint8_t error code - 0 on success
         */
        uint8_t getCurrentState();
        uint8_t setCurrentState(bool is_open, time_t epoch_s = 0);
};

/**
 * @brief Get door sensors from MQTT
 * 
 * @param payload MQTT payload
 * @param length length of payload
 * @return int8_t error code (0 on success)
 */
int8_t get_doors_mqtt(const byte* payload, const int length);

/**
 * @brief comparator for char* keys of our map of of OwensDoor's
 * 
 */
struct char_cmp { 
    bool operator () (const char *a,const char *b) const 
    {
        return strcmp(a,b)<0;
    } 
};

/**
 * @brief Provide a (pre-defined) map of description->door object
 * where "description" is a combination of door room-loc
 * 
 * @return std::map<const char*, OwensDoor> 
 */
std::map<const char*, OwensDoor, char_cmp> get_doors();

#endif /* FH_HOMESEC_H */