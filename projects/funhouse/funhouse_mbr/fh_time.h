#pragma once
#ifndef FH_TIME_H
#define FH_TIME_H

#include <time.h>

class FhNtpClient {
    private:
    public:
        /**
         * @brief Construct a new Fh Ntp Client object
         * 
         * @param update_interval - NTP update interval, seconds (optional)
         */
        FhNtpClient(uint16_t update_interval = 0);
        /**
         * @brief Get the time formatted as hh:mm:ss
         * 
         * @return String 
         */
        String getFormattedTime();
};

#endif /* FH_TIME_H */