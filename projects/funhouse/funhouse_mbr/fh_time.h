#pragma once
#ifndef FH_TIME_H
#define FH_TIME_H

/**
 * @brief Convert a string to epoch time, seconds
 * 
 * @param time_str String containing epoch timestamp; in sec, ms, us, or ns
 * 
 * @return uint32_t 
 */
uint32_t get_epoch_sec(const char* time_str);

uint8_t sec_to_string(char* time_str, uint32_t seconds);

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
        void getFormattedTime(char* outStr);
};

#endif /* FH_TIME_H */