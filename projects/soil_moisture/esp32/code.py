import time
import board
from analogio import AnalogIn
from ulab.numpy import interp

ADC_BITS = 16
V_REF = 3.3
"""
dry and wet values are empirically determined for each individual soil probe
dry: ADC value when probe in air
wet: ADC value when probe submerged in water
"""
probes = {
    "A0": {
        "analog_in": None,
        "bits_dry": 52586,
        "bits_wet": 29113
    }
}


def get_voltage(pin):
    """
    Get voltage value from an ADV pin
    """
    return (probes[pin]["analog_in"].value * V_REF) / ((1 << ADC_BITS) - 1)


def dry_pct(pin):
    """
    The percentage of "wet" into the probe's range
    """
    bits = probes[pin]["analog_in"].value
    dry_pct = interp(
        bits,
        [probes[pin]["bits_wet"], probes[pin]["bits_dry"]],
        [0, 100]
        )[0]
    return dry_pct


def wet_pct(pin):
    """
    The percentage of "wet" into the probe's range
    """
    return 100.0 - dry_pct(pin)


def main():
    # Connect to probes
    for pin in probes:
        probes[pin]["analog_in"] = AnalogIn(eval(f'board.{pin}'))
    while True:
        # print((get_voltage(analog_in),))
        # analog_in = AnalogIn(eval(f'board.{ADC_PIN}'))
        for pin in probes:
            bits = probes[pin]["analog_in"].value
            dry = dry_pct(pin)
            print(f"{pin}: {bits} bits, dry: {dry:.1f}%")
        time.sleep(5)


if __name__ == "__main__":
    main()
