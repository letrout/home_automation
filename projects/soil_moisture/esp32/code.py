import time
import board
from analogio import AnalogIn
from ulab.numpy import interp

ADC_BITS = 16
V_REF = 3.3
ADC_PIN = "A1"
# DRY and WET values are empirically determined for
# each individual soil probe
BITS_DRY = 52586    # ADC value when probe in air
BITS_WET = 29113    # ADC value when probe submersed in water


def get_voltage(pin):
    """
    Get voltage value from an ADV pi n
    """
    return (pin.value * V_REF) / ((1 << ADC_BITS) - 1)


def dry_pct(bits):
    """
    The percentage of "wet" into the probe's range
    """
    return interp(bits, [BITS_WET, BITS_DRY], [0, 100])[0]


def wet_pct(bits):
    """
    The percentage of "wet" into the probe's range
    """
    return 100.0 - dry_pct(bits)


def main():
    analog_in = AnalogIn(eval(f'board.{ADC_PIN}'))
    while True:
        # print((get_voltage(analog_in),))
        # analog_in = AnalogIn(eval(f'board.{ADC_PIN}'))
        bits = analog_in.value
        dry = dry_pct(bits)
        print(f"{ADC_PIN}: {bits} bits, dry: {dry:.1f}%")
        time.sleep(5)


if __name__ == "__main__":
    main()
