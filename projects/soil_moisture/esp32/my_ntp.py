"""
Simple NTP client for CircuitPython

from https://www.mattcrampton.com/blog/query_an_ntp_server_from_python/
"""

import rtc
import socketpool
import struct, time

import my_wifi
 
def getNTPTime(pool, host="pool.ntp.org"):
    """
    Get the current time from NTP server
    params:
        pool: socketpool
        host: NTP server hostname/IP
    return:
        t: seconds since 1970
    """
    port = 123
    buf = 48
    address = (host,port)
    msg = '\x1b' + 47 * '\0'
    recv_msg = bytearray(buf)
 
    # reference time (in seconds since 1900-01-01 00:00:00)
    TIME1970 = 2208988800 # 1970-01-01 00:00:00
 
    # connect to server
    client = pool.socket( pool.AF_INET, pool.SOCK_DGRAM)
    client.sendto(msg.encode('utf-8'), address)
    #msg, address = client.recv_into( recv_msg, buf )
    client.recv_into( recv_msg, buf )
 
    t = struct.unpack( "!12I", recv_msg )[10]
    t -= TIME1970
    #return time.ctime(t).replace("  "," ")
    return t

def set_rtc_to_ntp(pool, host="pool.ntp.org"):
    """
     Set the RTC to time from NTP server
    params:
        pool: socketpool
        host: NTP server hostname/IP
    return:
        nothing
    """
    t = getNTPTime(pool, host)
    rtc.RTC().datetime = time.localtime(t)
 
if __name__ == "__main__":
    my_wifi.connect()
    pool = socketpool.SocketPool(my_wifi.wifi.radio)
    t = getNTPTime(pool)
    print(time.localtime())
    rtc.RTC().datetime = time.localtime(t)
    print(time.localtime())
