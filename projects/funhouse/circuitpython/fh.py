
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


class MyFunHouse(object):
    """
    wrapper class for Adafruit FunHouse
    """
    def __init__(self, funhouse=None):
        self.__labels = {}
        if funhouse is None:
            self.funhouse = FunHouse(default_bg=BG_COLOR)
        else:
            self.funhouse = funhouse
        self.funhouse.peripherals.dotstars.fill(INITIAL_LIGHT_COLOR)
        self.status = Circle(229, 10, 10, fill=0xFF0000, outline=0x880000)
        self.funhouse.display.show(None)

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


class FunHouseDisplay(object):
    """
    A class to handle Adafruit Funhouse display
    """
    def __init__(self, funhouse):
        """
        Constructor
        param funhouse: a FunHouse object
        """
        self.__funhouse = funhouse
        self.__funhouse.peripherals.dotstars.fill(INITIAL_LIGHT_COLOR)
        self.status = Circle(229, 10, 10, fill=0xFF0000, outline=0x880000)
        # Don't display the splash yet to avoid
        # redrawing labels after each one is added
        self.__funhouse.display.show(None)
        # Add the labels
        self.__funhouse.add_text(
            text="Temperature:",
            text_position=(20, 30),
            text_color=0xFF8888,
            text_font="fonts/Arial-Bold-24.pcf",
        )
        self.__temp_label = funhouse.add_text(
            text_position=(120, 60),
            text_anchor_point=(0.5, 0.5),
            text_color=0xFFFF00,
            text_font="fonts/Arial-Bold-24.pcf",
        )
        self.__funhouse.add_text(
            text="Humidity:",
            text_position=(20, 100),
            text_color=0x8888FF,
            text_font="fonts/Arial-Bold-24.pcf",
        )
        self.__hum_label = funhouse.add_text(
            text_position=(120, 130),
            text_anchor_point=(0.5, 0.5),
            text_color=0xFFFF00,
            text_font="fonts/Arial-Bold-24.pcf",
        )
        self.__funhouse.add_text(
            text="Pressure:",
            text_position=(20, 170),
            text_color=0xFF88FF,
            text_font="fonts/Arial-Bold-24.pcf",
        )
        self.__pres_label = funhouse.add_text(
            text_position=(120, 200),
            text_anchor_point=(0.5, 0.5),
            text_color=0xFFFF00,
            text_font="fonts/Arial-Bold-24.pcf",
        )

        # Now display the splash to draw all labels at once
        self.__funhouse.display.show(self.__funhouse.splash)
        self.__funhouse.splash.append(self.status)

    @property
    def temp_label(self):
        return self.__temp_label

    @property
    def hum_label(self):
        return self.__hum_label

    @property
    def pres_label(self):
        return self.__pres_label


class FunHouseTextLabel(object):
    def __init__(self, fh, pos, anchor, color, font):
        self.fh = fh
        self.label = self.fh.add_text(
            text_position=pos,
            text_anchor_point=anchor,
            text_color=color,
            text_font=font,
        )

    def set_text(self, txt_string):
        self.fh.set_text(txt_string, self.label)
