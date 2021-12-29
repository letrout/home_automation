"""
Code te test NTP lib
rename to code.py for running on uC
"""

import socketpool
import rtc
import time

import my_ntp
import my_wifi


def main():
    my_wifi.connect()
    pool = socketpool.SocketPool(my_wifi.wifi.radio)
    ntp_sec = my_ntp.get_ntp_sec(pool)
    print("before ntp:\t%s " % time.localtime())
    #rtc.RTC().datetime = time.localtime(ntp_sec)
    if my_ntp.set_rtc_to_ntp(pool) == 0:
        print("ntp seconds %d   " % ntp_sec)
        print("after ntp:\t%s" % time.localtime())
    else:
        print("ERROR setting RTC to NTP")

if __name__ == "__main__":
    main()