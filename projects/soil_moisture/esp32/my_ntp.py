"""
Simple NTP client for CircuitPython

from https://www.mattcrampton.com/blog/query_an_ntp_server_from_python/
"""

import rtc
import struct
import time

import my_wifi


def get_ntp_sec(pool, host="pool.ntp.org"):
    """
    Get the current time from NTP server
    params:
        pool: socketpool
        host: NTP server hostname/IP
    return:
        t: seconds since 1970
    """

    if my_wifi.wifi.radio.ipv4_address is None:
        my_wifi.connect()
        if my_wifi.wifi.radio.ipv4_address is None:
            print("ERROR: NTP client failed to connect to WiFi")
            return None

    port = 123
    buf = 48
    address = (host, port)
    msg = '\x1b' + 47 * '\0'
    recv_msg = bytearray(buf)

    # reference time (in seconds since 1900-01-01 00:00:00)
    time_1970 = 2208988800 # 1970-01-01 00:00:00

    # connect to server
    try:
        client = pool.socket(pool.AF_INET, pool.SOCK_DGRAM)
        client.sendto(msg.encode('utf-8'), address)
        #msg, address = client.recv_into( recv_msg, buf )
        client.recv_into(recv_msg, buf)
    except:
        return None

    ntp_sec = struct.unpack("!12I", recv_msg)[10]
    ntp_sec -= time_1970
    #return time.ctime(t).replace("  "," ")
    return ntp_sec


def set_rtc_to_ntp(pool, host="pool.ntp.org"):
    """
     Set the RTC to time from NTP server
    params:
        pool: socketpool
        host: NTP server hostname/IP
    return:
        0: success
        1: failure
    """
    ntp_sec = get_ntp_sec(pool, host)
    if ntp_sec is not None:
        rtc.RTC().datetime = time.localtime(ntp_sec)
        return 0
    return 1
