/**
 * @file fh_homesec.cpp
 * @author Joel Luth (joel.luth@gmail.com)
 * @brief Classes to manage home security sensors on Adafruit Funhouse
 * @version 0.1
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <InfluxDbClient.h>
#include "fh_homesec.h"
#include "fh_mqtt.h"
#include "fh_time.h"
#include "secrets_homesec.h"

const uint8_t door_query_d = 7; // default door query range, in days
extern FhPubSubClient mqtt_client;
#ifdef FH_HOMESEC_H
extern std::map<const char*, OwensDoor, char_cmp> owensDoors;
#endif

InfluxDBClient influx_client(influxdb_url, influxdb_org, bucket_events, token_events);

OwensDoor::OwensDoor(const char* room, const char* loc) {
    strncpy(room_, room, ROOM_LOC_LEN);
    strncpy(loc_, loc, ROOM_LOC_LEN);
}

uint8_t OwensDoor::setCurrentState(bool is_open, time_t epoch_s) {
    is_open_ = is_open;
    if (epoch_s) {
        last_update_epoch_s_ = epoch_s;
    } else {
        time(&last_update_epoch_s_);
    }
    return 0;
}

uint8_t OwensDoor::secSinceOpen(uint32_t *seconds) {
    if (!influx_client.validateConnection()) {
        return 1;
    }
    uint32_t q_sec;
    char query[550];
    const char * result_name = "since_open";
    sprintf(query, "import \"system\""
    "currentTime = system.time()"
    "from(bucket: \"home_events\")"
    "|> range(start: -%dd)"
    "|> filter(fn: (r) => r[\"_measurement\"] == \"owens_events\")"
    "|> filter(fn: (r) => r[\"_field\"] ==  \"state\")"
    "|> filter(fn: (r) => r[\"type\"] == \"door\")"
    "|> filter(fn: (r) => r[\"room\"] == \"%s\")"
    "|> filter(fn: (r) => r[\"room_loc\"] == \"%s\")"
    "|> filter(fn: (r) => r[\"_value\"] == 1)"
    "|> last()"
    "|> map(fn: (r) => ({ %s: (uint(v: currentTime) - uint(v: r._time))/uint(v: 1000000000)}))"
    "|> yield(name: \"%s\")",
    door_query_d, room_, loc_, result_name, result_name);
    Serial.println(query);
    FluxQueryResult result = influx_client.query(query);
    if (result.getError() != "") {
        Serial.printf("Error getting door %s %s\n", room_, loc_);
        Serial.println(result.getError());
        Serial.println(query);
        return 2;
    }
    while (result.next()) {
        q_sec = result.getValueByName(result_name).getUnsignedLong();
        Serial.printf("Seconds: %lu\n", q_sec);
    }
    result.close();
    *seconds = q_sec;
    return 0;
}

uint8_t OwensDoor::getCurrentState() {
    if (!influx_client.validateConnection()) {
        return 1;
    }
    char query[512];
    bool state;
    unsigned long time_ms;
    sprintf(query, "from(bucket: \"%s\") |> range(start: -1h) |> filter(fn: (r) => r[\"_measurement\"] == \"owens_events\") |> filter(fn: (r) => r[\"room\"] == \"%s\") |> filter(fn: (r) => r[\"room_loc\"] == \"%s\") |> filter(fn: (r) => r[\"_field\"] == \"state\") |> last()", bucket_events, room_, loc_);
    Serial.println(query);
    FluxQueryResult result = influx_client.query(query);
    if (result.getError() != "") {
        Serial.printf("Error getting door %s %s\n", room_, loc_);
        Serial.println(result.getError());
        Serial.println(query);
        return 2;
    }
    while (result.next()) {
        state = result.getValueByName("_value").getDouble();
        FluxDateTime time = result.getValueByName("_time").getDateTime();
        time_ms = time.microseconds;
    }
    result.close();
    last_update_ms_ = time_ms;
    is_open_ = state;
    // Serial.printf("%s %s open %d, time %f\n", room_, loc_, state, time_ms);
    return 0;
}

std::map<const char*, OwensDoor, char_cmp> get_doors() {
    // FIXME: generate this with an array of room/loc and make_key()?
    std::map<const char*, OwensDoor, char_cmp> owensDoors = {
        {"garage-main", OwensDoor("garage", "main")},
        {"garage-side", OwensDoor("garage", "side")},
        {"mud-back", OwensDoor("mud", "back")},
        {"kitchen-deck", OwensDoor("kitchen", "deck")},
        {"library-front", OwensDoor("libray", "front")},
    };
    return owensDoors;
}

uint8_t OwensDoor::make_key(const char* room, const char* loc, char* key) {
    strncpy(key, room, strlen(room));
    strncat(key, "-", 1);
    strncat(key, loc, strlen(loc));
    return 0;
}

int8_t get_doors_mqtt(const byte* payload, const int length) {
    bool state;
    char msg[length + 1], key[ROOM_LOC_LEN * 2 + 1];
    char *pch, *ptr;
    char room[ROOM_LOC_LEN + 1], loc[ROOM_LOC_LEN + 1];
    uint32_t epoch_s = 0;
    memset(room, '\0', sizeof(room));
    memset(loc, '\0', sizeof(loc));
    memset(key, '\0', sizeof(key));
    //memccpy(msg, payload, sizeof(payload), sizeof(char));
    for (int i = 0; i < length; i++) {
        msg[i] = (char)payload[i];
        //msg[i + 1] = '\0';
    }
    msg[length] = '\0';
    //Serial.println(msg);
    //Get the room
    pch = strstr(msg, "room="); // payload starting at "room="
    if (pch != NULL) {
        pch = strtok(pch, "="); // split result on delimiters
    }
    if (pch != NULL) {
        //ptr = strtok(NULL, ",");  // get the second token after split
        strncpy(room, strtok(NULL, ","), ROOM_LOC_LEN);
    } else {
        return 1;
    }
    // Get the loc
    for (int i = 0; i < length; i++) {
        msg[i] = (char)payload[i];
        //msg[i + 1] = '\0';
    }
    msg[length] = '\0';
    pch = strstr(msg, "room_loc="); // payload starting at "room="
    if (pch != NULL) {
        pch = strtok(pch, "="); // split result on delimiters
    }
    if (pch != NULL) {
        //loc = strtok(NULL, ",");  // get the second token after split
        //loc = strtok(NULL, ",");
        //strcpy(loc, ptr);
        strncpy(loc, strtok(NULL, ","), ROOM_LOC_LEN);
    } else {
        return 2;
    }
    Serial.printf("ROOM: %s, LOC: %s\n", room, loc);
    if (OwensDoor::make_key(room, loc, key) !=0) {
        Serial.printf("ROOM: %s, LOC: %s, key: %s\n", room, loc, key);
        return 3;
    }
    if (owensDoors.count(key) != 1) {
        Serial.printf("key not found room: %s, loc: %s, key: %s, count: %d\n", room, loc, key, owensDoors.count(key));
        return 4;
    }
    // Get the door state
    for (int i = 0; i < length; i++) {
        msg[i] = (char)payload[i];
        //msg[i + 1] = '\0';
    }
    msg[length] = '\0';
    pch = strstr(msg, "state="); // payload starting at "room="
    if (pch != NULL) {
        pch = strtok(pch, "="); // split result on delimiters
    } else {
        return 5;
    }
    if (pch != NULL) {
        state = atoi(strtok(NULL, ","));  // get the second token after split
        //Serial.printf("%s state: %d\n", key, state);
    } else {
        return 6;
    }
    // Get the time
    for (int i = 0; i < length; i++) {
        msg[i] = (char)payload[i];
        //msg[i + 1] = '\0';
    }
    msg[length] = '\0';
    pch = strstr(msg, "state=");
    if (pch != NULL) {
        pch = strtok(pch, " ");
        if (pch != NULL ) {
            //time_ns = strtoul(strtok(NULL, " "), &ptr, 10);
            epoch_s = get_epoch_sec(strtok(NULL, " "));
            //Serial.printf("%s time: %lu\n", key, epoch_s);
        }
    }
    // Update the door object with the new data
    return owensDoors.at(key).setCurrentState(state, epoch_s);
}