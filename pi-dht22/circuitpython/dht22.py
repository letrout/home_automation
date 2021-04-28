# A script to poll DHT22 for temp/humidity
# TODO: publish to MQTT

import adafruit_dht
import board
import RPi.GPIO as GPIO
from datetime import datetime
import getopt
from os.path import dirname, join, abspath
import sys
import time

sys.path.insert(0, abspath(join(dirname(__file__), '../..')))
from lib.mqtt import mqtt_pub

#DHT_SENSOR = Adafruit_DHT.DHT22
PROBE_NAME = "PI4"
DEFAULT_INT = 2
DEFAULT_PIN = 4

def poll_dht22(dht_device):
    hum = None
    temp = None
    try:
        #hum, temp = adafruit_dht.read_retry(DHT_SENSOR, pin)
        temp = dht_device.temperature
        hum = dht_device.humidity
    except RuntimeError as error:
        print(error.args[0])
    return (hum, temp)

def print_loop(interval, dht_device):
    while True:
        hum, temp = poll_dht22(dht_device)
        if hum is not None and temp is not None:
            print(f'{datetime.now()} - T={c_to_f(temp):0.1f}F H={hum:0.1f}%')
        else:
            print("Failed to retrieve data from humidity sensor")
        #break
        time.sleep(interval)

def c_to_f(temp_c):
    return temp_c * (9 / 5) + 32

def main(argv):
    #GPIO.setmode(GPIO.BCM)
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

    dht_device = adafruit_dht.DHT22(eval(f'board.D{pin}'))
    if use_mqtt:
        hum, temp = poll_dht22(dht_device)
        m_client = mqtt_pub.client()
        if temp is not None:
            ret = mqtt_pub.publish(
                pin, 'temp_F', c_to_f(temp), m_client)
        if hum is not None:
            ret = mqtt_pub.publish(pin, 'rel_humidity', hum, m_client)
        mqtt_pub.disconnect(m_client)
    else:
        print_loop(interval, dht_device)

if __name__ == "__main__":
    main(sys.argv[1:])
