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

String FhNtpClient::getFormattedTime() {
    time_t now;
    tm tm;
    String hr, min, sec;
    time(&now);
    localtime_r(&now, &tm);
    hr = tm.tm_hour < 10 ? "0" + String(tm.tm_hour) : String(tm.tm_hour);
    min = tm.tm_min < 10 ? "0" + String(tm.tm_min) : String(tm.tm_min);
    sec = tm.tm_sec < 10 ? "0" + String(tm.tm_sec) : String(tm.tm_sec);
    return hr + ":" + min + ":" + sec;
}
