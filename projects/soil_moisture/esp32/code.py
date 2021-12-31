import time
import board
from analogio import AnalogIn
from ulab.numpy import interp

import my_influx
import my_mqtt
from secrets import influx, probes, secrets

ADC_BITS = 16
V_REF = 3.3


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


def publish_influx(pin, val, client=None, time_ns=None):
    """
    Publish to MQTT, with a message formatted in Influxdb2 line protocol
    """
    tags = influx["tags"]
    tags["plant"] = probes[pin]["plant"]
    fields = {}
    fields["wet_pct"] = val
    measurement = influx["_measurement"]
    lp = my_influx.influx_lp(measurement, fields, tags, time_ns)
    topic = secrets["mqtt_topic"]
    return my_mqtt.mqtt_publish(topic, lp, client)


def main():
    # Connect to probes
    for pin in probes:
        probes[pin]["analog_in"] = AnalogIn(eval(f'board.{pin}'))
    mqtt_client = my_mqtt.get_client()
    while True:
        # print((get_voltage(analog_in),))
        # analog_in = AnalogIn(eval(f'board.{ADC_PIN}'))
        for pin in probes:
            bits = probes[pin]["analog_in"].value
            wet = wet_pct(pin)
            print(f"{pin}: {bits} bits, wet: {wet:.1f}%")
            publish_influx(pin, wet, mqtt_client)
        time.sleep(5)


if __name__ == "__main__":
    main()
