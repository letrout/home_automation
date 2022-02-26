
"""
fh.py
A class to wrap Adafruit Funhouse
"""

__author__ = "Joel Luth"
__copyright__ = "Copyright 2022, Joel Luth"
__credits__ = ["Joel Luth"]
__license__ = "MIT"
__maintainer__ = "Joel Luth"
__email__ = "joel.luth@gmail.com"
__status__ = "Prototype"

import json
import time

from adafruit_display_shapes.circle import Circle
from adafruit_funhouse import FunHouse

try:
    from secrets import secrets
except ImportError:
    print("MQTT secrets are kept in secrets.py, please add them there!")
    raise

INITIAL_LIGHT_COLOR = 0x008000
BG_COLOR = 0x0F0F00
USE_FAHRENHEIT = True
TEMP_LABEL = "temp"
HUMIDITY_LABEL = "hum"
PRESSURE_LABEL = "pres"
SENSORS = {
    "temperature": ["aht20", "dp310", "scd40"],
    "humidity": ["aht20", "scd40"],
    "pressure": ["dp310"],
    "light": [],
    "pir": []
}


class MyFunHouse(object):
    """
    wrapper class for Adafruit FunHouse
    """
    def __init__(
            self, funhouse=None, temp=None, hum=None, press=None, enable_pir=False,
            topic_state=None, topic_ls=None, topic_lc=None):
        self.__labels = {}
        self.__environment = {}
        self.__temp = None
        self.__humidity = None
        self.__pressure = None
        self.__enable_pir = enable_pir
        self.__last_publish_timestamp = None
        self.__last_environment_timestamp = None
        self.__topic_state = topic_state
        self.__topic_light_state = topic_ls
        self.__topic_light_command = topic_lc
        if funhouse is None:
            self.funhouse = FunHouse(default_bg=BG_COLOR)
        else:
            self.funhouse = funhouse
        if temp in SENSORS["temperature"]:
            self.__temp = temp
        if hum in SENSORS["humidity"]:
            self.__humidity = hum
        if press in SENSORS["pressure"]:
            self.__pressure = press
        self.update_peripheral_state()
        self.funhouse.peripherals.dotstars.fill(INITIAL_LIGHT_COLOR)
        self.status = Circle(229, 10, 10, fill=0xFF0000, outline=0x880000)
        self.funhouse.splash.append(self.status)
        self.funhouse.display.show(None)
        # Initialize a new MQTT Client object
        self.funhouse.network.init_mqtt(
            secrets["mqtt_broker"],
            secrets["mqtt_port"],
            secrets["mqtt_user"],
            secrets["mqtt_password"],
        )
        self.funhouse.network.on_mqtt_connect = self.connected
        self.funhouse.network.on_mqtt_disconnect = self.disconnected
        self.funhouse.network.on_mqtt_message = self.message
        print("Attempting to connect to {}".format(secrets["mqtt_broker"]))
        self.funhouse.network.mqtt_connect()

    @property
    def environment(self):
        return self.__environment

    @property
    def last_peripheral_state(self):
        return self.__last_peripheral_state

    @property
    def last_environment_timestamp(self):
        return self.__last_environment_timestamp

    @property
    def last_publish_timestamp(self):
        return self.__last_publish_timestamp

    @property
    def read_temp_c(self):
        if self.__temp == "aht20":
            temp = self.funhouse.peripherals.temperature
            return temp
        return None

    @property
    def read_relative_humidity(self):
        if self.__humidity == "aht20":
            hum = self.funhouse.peripherals.relative_humidity
            return hum
        return None

    @property
    def read_pressure_hpa(self):
        if self.__pressure == "dp310":
            press = self.funhouse.peripherals.pressure
            return press
        return None

    def label(self, label_name):
        try:
            return self.__labels[label_name]
        except KeyError:
            return None

    def set_label(
            self, label_name,
            text_position,
            text_anchor_point,
            text_color,
            text_font
            ):
        self.__labels[label_name] = self.funhouse.add_text(
            text_position=text_position,
            text_anchor_point=text_anchor_point,
            text_color=text_color,
            text_font=text_font,
        )

    def redraw_display(self):
        self.funhouse.display.show(self.funhouse.splash)

    def update_enviro(self):
        temp = self.read_temp_c
        unit = "C"
        if USE_FAHRENHEIT:
            temp = temp * (9 / 5) + 32
            unit = "F"

        self.__environment["temperature"] = temp
        self.__environment["pressure"] = self.read_pressure_hpa
        self.__environment["humidity"] = self.read_relative_humidity
        self.__environment["light"] = self.funhouse.peripherals.light
        self.__last_environment_timestamp = time.monotonic()

        self.funhouse.set_text(
            "{:.1f}{}".format(self.__environment["temperature"], unit),
            self.label(TEMP_LABEL))
        self.funhouse.set_text(
            "{:.1f}%".format(self.__environment["humidity"]),
            self.label(HUMIDITY_LABEL))
        self.funhouse.set_text(
            "{:.0f}hPa".format(self.__environment["pressure"]),
            self.label(PRESSURE_LABEL))
        self.redraw_display()

    def update_peripheral_state(self):
        self.__last_peripheral_state = {
            "button_up": self.funhouse.peripherals.button_up,
            "button_down": self.funhouse.peripherals.button_down,
            "button_sel": self.funhouse.peripherals.button_sel,
            "captouch6": self.funhouse.peripherals.captouch6,
            "captouch7": self.funhouse.peripherals.captouch7,
            "captouch8": self.funhouse.peripherals.captouch8,
            }
        if self.__enable_pir:
            self.__last_peripheral_state["pir_sensor"] = \
                self.funhouse.peripherals.pir_sensor

    def set_peripheral_state(self, periph, state):
        previous = None
        if periph in self.__last_peripheral_state:
            previous = self.__last_peripheral_state[periph]
            self.__last_peripheral_state[periph] = state
        return previous

    def connected(self, client, userdata, result, payload):
        # FIXME: how to access fh status
        self.status.fill = 0x00FF00
        self.status.outline = 0x008800
        print("Connected to MQTT! Subscribing...")
        client.subscribe(self.__topic_light_command)

    def disconnected(self, client):
        # FIXME: how to access fh status
        self.status.fill = 0xFF0000
        self.status.outline = 0x880000
        pass

    def message(self, client, topic, payload):
        print("Topic {0} received new value: {1}".format(topic, payload))
        if topic == self.__topic_light_command:
            settings = json.loads(payload)
            if settings["state"] == "on":
                if "brightness" in settings:
                    self.funhouse.peripherals.dotstars.brightness = \
                        settings["brightness"] / 255
                else:
                    self.funhouse.peripherals.dotstars.brightness = 0.3
                if "color" in settings:
                    self.funhouse.peripherals.dotstars.fill(settings["color"])
            else:
                self.funhouse.peripherals.dotstars.brightness = 0
            self.publish_light_state()

    def publish_state(self, output):
        self.funhouse.peripherals.led = True
        print("Publishing to {}".format(self.__topic_state))
        self.funhouse.network.mqtt_publish(self.__topic_state, json.dumps(output))
        self.funhouse.peripherals.led = False
        self.__last_publish_timestamp = time.monotonic()

    def publish_light_state(self):
        self.funhouse.peripherals.led = True
        output = {
            "brightness": round(self.funhouse.peripherals.dotstars.brightness * 255),
            "state": "on" if self.funhouse.peripherals.dotstars.brightness > 0 else "off",
            "color": self.funhouse.peripherals.dotstars[0],
        }
        # Publish the Dotstar State
        print("Publishing to {}".format(self.__topic_light_state))
        self.funhouse.network.mqtt_publish(self.__topic_light_state, json.dumps(output))
        self.funhouse.peripherals.led = False
