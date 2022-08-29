#ifndef FH_TIME_H
#define FH_TIME_H

#include <NTPClient.h>

class FhNtpClient : public NTPClient {
    private:
    public:
        /**
         * @brief Construct a new Fh Ntp Client object
         * uses const char* ntp_server, long utcOffsetInSeconds from secrets.h
         * 
         * @param udp WiFiUDP object
         * @param updateInterval Update interval in ms
         */
        FhNtpClient(UDP& udp);
        FhNtpClient(UDP& udp, unsigned long updateInterval);
};

#endif /* FH_TIME_H */