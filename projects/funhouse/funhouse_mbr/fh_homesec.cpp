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