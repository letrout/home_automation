import time
import board
from analogio import AnalogIn
from ulab.numpy import interp

ADC_BITS = 16
V_REF = 3.3
ADC_PIN = "A1"
# DRY and WET values are empirically determined for
# each individual soil probe
BITS_DRY = 52586 # ADC value when probe in air
BITS_WET = 29113 # ADC value when probe submersed in water

analog_in = AnalogIn(eval(f'board.{ADC_PIN}'))
#analog_in = AnalogIn(board.A1)


def get_voltage(pin):
    return (pin.value * V_REF) / ((1 << ADC_BITS) -1)


def dry_pct(bits):
    return interp(bits, [BITS_WET, BITS_DRY], [0,100])[0]


def wet_pct(bits):
    return 100.0 - dry_pct(bits)


while True:
    #print((get_voltage(analog_in),))
    #analog_in = AnalogIn(eval(f'board.{ADC_PIN}'))
    bits = analog_in.value
    dry = dry_pct(bits)
    print(f"{ADC_PIN}: {bits} bits, dry: {dry:.1f}%")
    time.sleep(5)

