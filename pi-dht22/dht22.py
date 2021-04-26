# A script to poll DHT22 for temp/humidity
# FIXME: convert to adafruit-circuitpython-dht
# TODO: publish to MQTT

import Adafruit_DHT
from datetime import datetime
import getopt
import sys
import time

DHT_SENSOR = Adafruit_DHT.DHT22
PROBE_NAME = "PI4"
DEFAULT_INT = 2
DEFAULT_PIN = 4

def poll_dht22(sensor, pin):
    hum = None
    temp = None
    try:
        hum, temp = Adafruit_DHT.read_retry(DHT_SENSOR, pin)
    except RuntimeError as error:
        print(error.args[0])
    return (hum, temp)

def print_loop(interval, pin):
    while True:
        hum, temp = poll_dht22(DHT_SENSOR, pin)
        if hum is not None and temp is not None:
            print(f'{datetime.now()} - T={c_to_f(temp):0.1f}F H={hum:0.1f}%')
        else:
            print("Failed to retrieve data from humidity sensor")
        time.sleep(interval)

def c_to_f(temp_c):
    return temp_c * (9 / 5) + 32

def main(argv):
    interval = DEFAULT_INT
    pin = DEFAULT_PIN
    try:
        opts, args = getopt.getopt(argv,"hi:p:",["interval=","pin="])
    except getopt.GetoptError:
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print('dht22.py -s <seconds between readings> -p <GPIO pin>')
            sys.exit()
        elif opt in ("-i", "--interval"):
            interval = int(arg)
        elif opt in ("-p", "--pin"):
            pin = int(arg)

    print_loop(interval, pin)

if __name__ == "__main__":
    main(sys.argv[1:])
