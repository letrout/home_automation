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

OwensDoor owensDoors[] = {OwensDoor("garage", "main"),
                            OwensDoor("garage", "side"),
                            OwensDoor("mud", "back"),
                            OwensDoor("kitchen", "deck"),
                            OwensDoor("library", "front")};

OwensDoor::OwensDoor(const char* room, const char* loc) {
    strncpy(room_, room, ROOM_LOC_LEN);
    strncpy(loc_, loc, ROOM_LOC_LEN);
}
