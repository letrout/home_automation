import Adafruit_DHT
import time
from datetime import datetime

DHT_SENSOR = Adafruit_DHT.DHT22
DHT_PIN = 4
PROBE_NAME = "PI4"

def print_loop(sleep_sec=2):
    while True:
        try:
            hum, temp = Adafruit_DHT.read_retry(DHT_SENSOR, DHT_PIN)
        except RuntimeError as error:
            print(error.args[0])
            continue
        except Exception as error:
            raise error

        if hum is not None and temp is not None:
            print(f'{datetime.now()} - T={c_to_f(temp):0.1f}F H={hum:0.1f}%')
        else:
            print("Failed to retrieve data from humidity sensor")
        time.sleep(sleep_sec)

def c_to_f(temp_c):
    return temp_c * (9 / 5) + 32

if __name__ == "__main__":
    print_loop()
