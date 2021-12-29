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
    t = my_ntp.getNTPTime(pool)
    print(time.localtime())
    rtc.RTC().datetime = time.localtime(t)
    print(time.localtime())

if __name__ == "__main__":
    main()