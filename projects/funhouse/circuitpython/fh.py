
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
