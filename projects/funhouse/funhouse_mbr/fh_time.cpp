#include "Arduino.h"
#include "fh_time.h"
#include "esp_sntp.h"

#define MIN_NTP_UPDATE_SEC 15   // minimum NTP update interval, sec
#define EPOCH_SEC_DIGITS 10     // digits in epoch seconds timestamp

// Define NTP Client to get time
const long utcOffsetInSeconds = 0;
const unsigned long ntp_update_ms = 30 * 60 * 1000L; // NTP update interval ms
const char* ntp_server = "ntp.luth.bog";
const char* fh_tz = "CST6CDT,M3.2.0,M11.1.0";   // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv

FhNtpClient ntp_client;

uint32_t get_epoch_sec(const char *time_str) {
    char *remainder, my_time_str[EPOCH_SEC_DIGITS + 1];
    uint32_t epoch_sec;
    my_time_str[0] = '\0';
    strncat(my_time_str, time_str, EPOCH_SEC_DIGITS);
    // TODO: check if string is all digits
    epoch_sec = strtoul(my_time_str, &remainder, 10);
    return epoch_sec;
}

// NTP client
// FIXME: add option to set update interval
FhNtpClient::FhNtpClient(uint16_t update_interval) {
    if (update_interval > MIN_NTP_UPDATE_SEC) {
        sntp_set_sync_interval(update_interval * 1000UL);
    }
    configTime(utcOffsetInSeconds, 0, ntp_server);
    setenv("TZ", fh_tz, 1);
    tzset();
}

void FhNtpClient::getFormattedTime(char* outStr) {
    char timeStr[9];
    time_t now;
    tm tm;
    time(&now);
    localtime_r(&now, &tm);
    sprintf(timeStr, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
    // TODO: something to prevent possible overflow (outStr too small)
    strncpy(outStr, timeStr, sizeof(timeStr));
}
