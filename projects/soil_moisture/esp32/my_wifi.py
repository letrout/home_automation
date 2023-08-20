"""
WiFi helper code for CircuitPython microcontrollers
"""

import ipaddress
import wifi

# Get wifi details and more from a secrets.py file
try:
    from secrets import secrets
except ImportError:
    print("WiFi secrets are kept in secrets.py, please add them there!")
    raise


def connect():
    """
    Connect to Wifi
    """
    retries = 5;
    if wifi.radio.ipv4_address is not None:
        print("Connected to %s, RSSI: %d!" % (str(wifi.radio.ap_info.ssid), wifi.radio.ap_info.rssi))
        return 0
    print("Connecting to %s"%secrets["ssid"])
    wifi.radio.connect(secrets["ssid"], secrets["password"])
    if wifi.radio.ipv4_address is not None:
        print("Connected to %s, RSSI: %d!" % (str(wifi.radio.ap_info.ssid), wifi.radio.ap_info.rssi))
        return 0
    print("FAILED to connect to %s!"%secrets["ssid"])
    return 1


def scan_networks():
    """
    Scan all visible WiFi networks and print results
    """
    print("Available WiFi networks:")
    for network in wifi.radio.start_scanning_networks():
        print("\t%s\t\tRSSI: %d\tChannel: %d" % (str(network.ssid, "utf-8"),
        network.rssi, network.channel))
    wifi.radio.stop_scanning_networks()


def get_ip():
    """
    Get my IP address
    """
    return wifi.radio.ipv4_address


def get_mac():
    """
    Get my MAC
    """
    return [hex(i) for i in wifi.radio.mac_address]


def ping(ipv4_address):
    """
    Ping a ipv4 address
    """
    ipv4 = ipaddress.ip_address(ipv4_address)
    print("Ping %s: %f ms" % (ipv4_address, wifi.radio.ping(ipv4)*1000))
