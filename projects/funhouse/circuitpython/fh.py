
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

from adafruit_display_shapes.circle import Circle
from adafruit_funhouse import FunHouse

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
    def __init__(self, funhouse=None, temp=None, hum=None, press=None):
        self.__labels = {}
        self.__environment = {}
        self.__temp = None
        self.__humidity = None
        self.__pressure = None
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
        self.funhouse.peripherals.dotstars.fill(INITIAL_LIGHT_COLOR)
        self.status = Circle(229, 10, 10, fill=0xFF0000, outline=0x880000)
        self.funhouse.display.show(None)

    @property
    def environment(self):
        return self.__environment

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
        # self.funhouse.splash.append(self.status)

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

        self.funhouse.set_text(
            "{:.1f}{}".format(self.__environment["temperature"], unit),
            self.label(TEMP_LABEL))
        self.funhouse.set_text(
            "{:.1f}%".format(self.__environment["humidity"]),
            self.label(HUMIDITY_LABEL))
        self.funhouse.set_text(
            "{}hPa".format(self.__environment["pressure"]),
            self.label(PRESSURE_LABEL))
        self.redraw_display()
