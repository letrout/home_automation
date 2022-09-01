#include "Arduino.h"
#include "fh_time.h"
#include "esp_sntp.h"

#define MIN_NTP_UPDATE_SEC 15   // minimum NTP update interval, sec

// Define NTP Client to get time
const long utcOffsetInSeconds = 0;
const unsigned long ntp_update_ms = 30 * 60 * 1000L; // NTP update interval ms
const char* ntp_server = "ntp.luth.bog";
const char* fh_tz = "CST6CDT,M3.2.0,M11.1.0";   // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv

FhNtpClient ntp_client;

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
