#pragma once
#ifndef FH_TIME_H
#define FH_TIME_H

#include <time.h>

class FhNtpClient {
    private:
    public:
        /**
         * @brief Construct a new Fh Ntp Client object
         */
        FhNtpClient();
        /**
         * @brief Get the time formatted as hh:mm:ss
         * 
         * @return String 
         */
        String getFormattedTime();
};

#endif /* FH_TIME_H */