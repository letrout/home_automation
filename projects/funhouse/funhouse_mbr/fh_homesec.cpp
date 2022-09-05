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
#include "secrets_homesec.h"

extern FhPubSubClient mqtt_client;

InfluxDBClient influx_client(influxdb_url, influxdb_org, bucket_events, token_events);

OwensDoor owensDoors[] = {OwensDoor("garage", "main"),
                            OwensDoor("garage", "side"),
                            OwensDoor("mud", "back"),
                            OwensDoor("kitchen", "deck"),
                            OwensDoor("library", "front")};

OwensDoor::OwensDoor(const char* room, const char* loc) {
    strncpy(room_, room, ROOM_LOC_LEN);
    strncpy(loc_, loc, ROOM_LOC_LEN);
}

uint8_t OwensDoor::getCurrentStateMqtt() {
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

int8_t get_doors_mqtt(const byte* payload, const int length) {
    int8_t ret = 0;
    bool state;
    char msg[length];
    char *pch, *room, *loc, *ptr;
    unsigned long time_ns;
    //memccpy(msg, payload, sizeof(payload), sizeof(char));
    for (int i = 0; i < length; i++) {
        msg[i] = (char)payload[i];
    }
    //Get the room
    pch = strstr(msg, "room="); // payload starting at "room="
    if (pch != NULL) {
        pch = strtok(pch, "="); // split result on delimiters
    }
    if (pch != NULL) {
        room = strtok(NULL, ",");  // get the second token after split
    }
    // Get the loc
    for (int i = 0; i < length; i++) {
        msg[i] = (char)payload[i];
    }
    pch = strstr(msg, "room_loc="); // payload starting at "room="
    if (pch != NULL) {
        pch = strtok(pch, "="); // split result on delimiters
    }
    if (pch != NULL) {
        loc = strtok(NULL, ",");  // get the second token after split
    }
    // Get the door state
    for (int i = 0; i < length; i++) {
        msg[i] = (char)payload[i];
    }
    pch = strstr(msg, "state="); // payload starting at "room="
    if (pch != NULL) {
        pch = strtok(pch, "="); // split result on delimiters
    }
    if (pch != NULL) {
        state = atoi(strtok(NULL, ","));  // get the second token after split
    }
    // Get the time
    for (int i = 0; i < sizeof(payload); i++) {
        msg[i] = (char)payload[i];
    }
    pch = strstr(msg, "state=");
    if (pch != NULL) {
        pch = strtok(pch, " ");
        if (pch != NULL ) {
            time_ns = strtoul(strtok(NULL, " "), &ptr, 10);
        }
    }
    return ret;
}