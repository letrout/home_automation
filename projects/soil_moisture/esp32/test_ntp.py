"""
Code te test NTP lib
rename to code.py for running on uC
"""

import socketpool
import time

import my_ntp
import my_wifi


def main():
    my_wifi.connect()
    pool = socketpool.SocketPool(my_wifi.wifi.radio)
    ntp_sec = my_ntp.get_ntp_sec(pool)
    print("before ntp:\t%s " % time.localtime())
    if my_ntp.set_rtc_to_ntp(pool) == 0:
        print("ntp seconds %d   " % ntp_sec)
        print("after ntp:\t%s" % time.localtime())
    else:
        print("ERROR setting RTC to NTP")
    time.sleep(1)
    print("time_ns from RTC: %d" % my_ntp.time_ns())
    time.sleep(1)
    print("time_ns from NTP client: %d" % my_ntp.time_ns(pool, "pool.ntp.org"))


if __name__ == "__main__":
    main()
