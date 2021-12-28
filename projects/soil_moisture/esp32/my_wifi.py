import ipaddress
import wifi

# Get wifi details and more from a secrets.py file
try:
    from secrets import secrets
except ImportError:
    print("WiFi secrets are kept in secrets.py, please add them there!")
    raise


def connect():
    print("Connecting to %s"%secrets["ssid"])
    wifi.radio.connect(secrets["ssid"], secrets["password"])
    print("Connected to %s!"%secrets["ssid"])


def scan_networks():
    print("Available WiFi networks:")
    for network in wifi.radio.start_scanning_networks():
        print("\t%s\t\tRSSI: %d\tChannel: %d" % (str(network.ssid, "utf-8"),
        network.rssi, network.channel))
    wifi.radio.stop_scanning_networks()


def get_ip():
    return wifi.radio.ipv4_address


def get_mac():
    return [hex(i) for i in wifi.radio.mac_address]


def ping(ipv4_address):
    ipv4 = ipaddress.ip_address(ipv4_address)
    print("Ping %s: %f ms" % (ipv4_address, wifi.radio.ping(ipv4)*1000))

