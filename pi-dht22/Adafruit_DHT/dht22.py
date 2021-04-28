# A script to poll DHT22 for temp/humidity
# FIXME: convert to adafruit-circuitpython-dht
# TODO: publish to MQTT

import Adafruit_DHT
from datetime import datetime
import getopt
from os.path import dirname, join, abspath
import sys
import time

sys.path.insert(0, abspath(join(dirname(__file__), '../..')))
from lib.mqtt import mqtt_pub

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
    use_mqtt = False
    try:
        opts, args = getopt.getopt(argv,"hi:p:m",["interval=","pin="])
    except getopt.GetoptError:
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print('dht22.py -i|--interval <seconds between readings>')
            print('-p|--pin <GPIO pin>')
            print('-m|--mqtt publish (once) to MQTT broker')
            sys.exit()
        elif opt in ("-i", "--interval"):
            interval = int(arg)
        elif opt in ("-p", "--pin"):
            pin = int(arg)
        elif opt in ("-m", "--mqtt"):
            use_mqtt = True

    if use_mqtt:
        hum, temp = poll_dht22(DHT_SENSOR, pin)
        m_client = mqtt_pub.client()
        if temp is not None:
            ret = mqtt_pub.publish(
                pin, 'temp_F', c_to_f(temp), m_client)
        time.sleep(5)
        if hum is not None:
            ret = mqtt_pub.publish(pin, 'rel_humidity', hum, m_client)
        mqtt_pub.disconnect(m_client)
    else:
        print_loop(interval, pin)

if __name__ == "__main__":
    main(sys.argv[1:])
