#include <WiFiUdp.h>
#include "fh_time.h"

// Define NTP Client to get time
const long utcOffsetInSeconds = 0;
const unsigned long ntp_update_ms = 30 * 60 * 1000L; // NTP update interval ms
const char* ntp_server = "ntp.luth.bog";
const char* fh_tz = "CST6CDT,M3.2.0,M11.1.0";   // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv

WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, ntp_server, utcOffsetInSeconds, ntp_update_ms);
FhNtpClient timeClient(ntpUDP, ntp_update_ms);

// NTP client
FhNtpClient::FhNtpClient(UDP& udp) : NTPClient(udp, ntp_server, utcOffsetInSeconds) {
  setenv("TZ", fh_tz, 1);
  tzset();
}

FhNtpClient::FhNtpClient(UDP& udp, unsigned long updateInterval) : NTPClient(udp, ntp_server, utcOffsetInSeconds, updateInterval) {
  setenv("TZ", fh_tz, 1);
  tzset();
}
