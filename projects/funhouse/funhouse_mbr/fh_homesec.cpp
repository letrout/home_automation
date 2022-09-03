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
#include "secrets_homesec.h"

InfluxDBClient client(influxdb_url, influxdb_org, bucket_events, token_events);

OwensDoor owensDoors[] = {OwensDoor("garage", "main"),
                            OwensDoor("garage", "side"),
                            OwensDoor("mud", "back"),
                            OwensDoor("kitchen", "deck"),
                            OwensDoor("library", "front")};

OwensDoor::OwensDoor(const char* room, const char* loc) {
    strncpy(room_, room, ROOM_LOC_LEN);
    strncpy(loc_, loc, ROOM_LOC_LEN);
}

uint8_t OwensDoor::getCurrentState() {
    if (!client.validateConnection()) {
        return 1;
    }
    char query[512];
    bool state;
    unsigned long time_ms;
    sprintf(query, "from(bucket: \"%s\") |> range(start: -1h) |> filter(fn: (r) => r[\"_measurement\"] == \"owens_events\") |> filter(fn: (r) => r[\"room\"] == \"%s\") |> filter(fn: (r) => r[\"room_loc\"] == \"%s\") |> filter(fn: (r) => r[\"_field\"] == \"state\") |> last()", bucket_events, room_, loc_);
    FluxQueryResult result = client.query(query);
    if (result.getError()) {
        Serial.println(result.getError());
        return 2;
    }
    while (result.next()) {
        state = result.getValueByName("_value").getBool();
        FluxDateTime time = result.getValueByName("_time").getDateTime();
        time_ms = time.microseconds / 1000;
    }
    result.close();
    last_update_ms_ = time_ms;
    is_open_ = state;
    return 0;
}