# A script to poll PMSA003I sensor for particulate counts

from datetime import datetime
import getopt
from os.path import dirname, join, abspath
import sys
import time
import board
import busio
from digitalio import DigitalInOut, Direction, Pull
from adafruit_pm25.i2c import PM25_I2C

sys.path.insert(0, abspath(join(dirname(__file__), '..')))
from lib.mqtt import mqtt_pub

DEFAULT_INT = 2
DEFAULT_PIN = 4

def poll_pmsa003i():
    reset_pin = None
    # If you have a GPIO, its not a bad idea to connect it to the RESET pin
    # reset_pin = DigitalInOut(board.G0)
    # reset_pin.direction = Direction.OUTPUT
    # reset_pin.value = False

    # Create library object, use 'slow' 100KHz frequency!
    i2c = busio.I2C(board.SCL, board.SDA, frequency=100000)
    # Connect to a PM2.5 sensor over I2C
    pm25 = PM25_I2C(i2c, reset_pin)
    try:
        aqdata = pm25.read()
    except RuntimeError:
        #print("Unable to read from sensor")
        return None
    return aqdata

def print_keys():
    aqdata = poll_pmsa003i()
    if aqdata is not None:
        print(sorted(aqdata.keys()))
    else:
        print("Unable to read from sensor")

def print_loop(interval):
    while True:
        aqdata = poll_pmsa003i()
        if aqdata is not None:
            print_aqdata(aqdata)
        else:
            print("Unable to read from sensor")
        time.sleep(interval)

def print_aqdata(aqdata):
    print("Concentration Units (standard)")
    print("---------------------------------------")
    print(
        f"PM 1.0: {aqdata['pm10 standard']}" +
        f"\tPM2.5: {aqdata['pm25 standard']}" + 
        f"\tPM10: {aqdata['pm100 standard']}"
        )
    print("Concentration Units (environmental)")
    print("---------------------------------------")
    print(
        f"PM 1.0: {aqdata['pm10 env']}" +
        f"\tPM2.5: {aqdata['pm25 env']}" + 
        f"\tPM10: {aqdata['pm100 env']}"
        )
    print("---------------------------------------")
    print(f"Particles > 0.3um / 0.1L air: {aqdata['particles 03um']}")
    print(f"Particles > 0.5um / 0.1L air: {aqdata['particles 05um']}")
    print(f"Particles > 1.0um / 0.1L air: {aqdata['particles 10um']}")
    print(f"Particles > 2.5um / 0.1L air: {aqdata['particles 25um']}")
    print(f"Particles > 5.0um / 0.1L air: {aqdata['particles 50um']}")
    print(f"Particles > 10 um / 0.1L air: {aqdata['particles 100um']}")
    print("---------------------------------------")

def main(argv):
    interval = DEFAULT_INT
    use_mqtt = False
    try:
        opts, args = getopt.getopt(argv,"hi:md",["interval="])
    except getopt.GetoptError:
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print('pmsa003i.py -i|--interval <seconds between readings>')
            print('-m|--mqtt publish (once) to MQTT broker')
            sys.exit(0)
        elif opt in ("-d", "--debug"):
            print_keys()
            sys.exit(0)
        elif opt in ("-i", "--interval"):
            interval = int(arg)
        elif opt in ("-m", "--mqtt"):
            use_mqtt = True

    if use_mqtt:
        aqdata = poll_pmsa003i()
        if aqdata is None:
            sys.exit(1)
        m_client = mqtt_pub.client()
        try:
            ret = mqtt_pub.publish(DEFAULT_PIN, 'PM1.0_std', aqdata['pm10 standard'], m_client)
            time.sleep(2)
            ret = mqtt_pub.publish(DEFAULT_PIN, 'PM2.5_std', aqdata['pm25 standard'], m_client)
            time.sleep(2)
            ret = mqtt_pub.publish(DEFAULT_PIN, 'PM10.0_std', aqdata['pm100 standard'], m_client)
            #time.sleep(10)
            #ret = mqtt_pub.publish(pin, 'rel_humidity', hum, m_client)
        except KeyError:
            print("key error")
        mqtt_pub.disconnect(m_client)
    else:
        print_loop(interval)


if __name__ == "__main__":
    main(sys.argv[1:])
