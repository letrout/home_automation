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
        "PM 1.0: %d\tPM2.5: %d\tPM10: %d"
        % (aqdata["pm10 standard"], aqdata["pm25 standard"], aqdata["pm100 standard"])
    )
    print("Concentration Units (environmental)")
    print("---------------------------------------")
    print(
        "PM 1.0: %d\tPM2.5: %d\tPM10: %d"
        % (aqdata["pm10 env"], aqdata["pm25 env"], aqdata["pm100 env"])
    )
    print("---------------------------------------")
    print("Particles > 0.3um / 0.1L air:", aqdata["particles 03um"])
    print("Particles > 0.5um / 0.1L air:", aqdata["particles 05um"])
    print("Particles > 1.0um / 0.1L air:", aqdata["particles 10um"])
    print("Particles > 2.5um / 0.1L air:", aqdata["particles 25um"])
    print("Particles > 5.0um / 0.1L air:", aqdata["particles 50um"])
    print("Particles > 10 um / 0.1L air:", aqdata["particles 100um"])
    print("---------------------------------------")

def main(argv):
    interval = DEFAULT_INT
    use_mqtt = False
    try:
        opts, args = getopt.getopt(argv,"hi:m",["interval="])
    except getopt.GetoptError:
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print('pmsa003i.py -i|--interval <seconds between readings>')
            print('-m|--mqtt publish (once) to MQTT broker')
            sys.exit()
        elif opt in ("-i", "--interval"):
            interval = int(arg)
        elif opt in ("-m", "--mqtt"):
            use_mqtt = True

    if use_mqtt:
        hum, temp = poll_dht22(pin)
        m_client = mqtt_pub.client()
        if temp is not None:
            ret = mqtt_pub.publish(
                pin, 'temp_F', c_to_f(temp), m_client)
        if hum is not None:
            # Without the delay, sometimes fail to see humidity post to broker
            time.sleep(10)
            ret = mqtt_pub.publish(pin, 'rel_humidity', hum, m_client)
        mqtt_pub.disconnect(m_client)
    else:
        print_loop(interval)


if __name__ == "__main__":
    main(sys.argv[1:])
