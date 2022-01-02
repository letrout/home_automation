import time
import board
import socketpool
from analogio import AnalogIn
from ulab.numpy import interp

import my_influx
import my_mqtt
import my_ntp
import my_wifi
from secrets import influx, probes, secrets

ADC_BITS = 16
V_REF = 3.3
NTP_SERVER = None  # If !None, set RTC from NTP and pass timestamp in MQTT msgs
QUERY_INT = 5  # Seconds between queries of the probes
MQTT_PUB = False  # Set to True to publish to MQTT
SAMPLES = 1  # The number of probe readings to take and average for each measurement


def get_voltage(pin):
    """
    Get voltage value from an ADV pin
    """
    return (probes[pin]["analog_in"].value * V_REF) / ((1 << ADC_BITS) - 1)


def dry_pct(pin, samples=1):
    """
    The percentage of "dry" into the probe's range
    params:
        pin - the pin number (key of dict in secrets)
        samples - the number of samples to take and average
    returns:
        dry_pct: the percentage of dry of the probe range
    """
    bits = 0
    for i in range(0,samples):
        bits += probes[pin]["analog_in"].value
    bits /= samples
    dry_pct = interp(
        bits,
        [probes[pin]["bits_wet"], probes[pin]["bits_dry"]],
        [0, 100]
        )[0]
    return dry_pct


def wet_pct(pin, samples=1):
    """
    The percentage of "wet" into the probe's range
    params:
        pin - the pin number (key of dict in secrets)
        samples - the number of samples to take and average
    returns:
        dry_pct: the percentage of wet of the probe range
    """
    return 100.0 - dry_pct(pin, samples)


def publish_influx(pin, val, client=None, time_ns=None, get_time=False):
    """
    Publish to MQTT, with a message formatted in Influxdb2 line protocol
    """
    tags = influx["tags"]
    tags["plant"] = probes[pin]["plant"]
    fields = {}
    fields["wet_pct"] = val
    measurement = influx["_measurement"]
    lp = my_influx.influx_lp(measurement, fields, tags, time_ns, get_time)
    topic = secrets["mqtt_topic"]
    return my_mqtt.mqtt_publish(topic, lp, client)


def main():
    mqtt_client = None
    # Connect to probes
    for pin in probes:
        probes[pin]["analog_in"] = AnalogIn(eval(f'board.{pin}'))
    if MQTT_PUB:
            mqtt_client = my_mqtt.get_client()
    while True:
        # print((get_voltage(analog_in),))
        # analog_in = AnalogIn(eval(f'board.{ADC_PIN}'))
        # Optionally, set RTC to NTP and use local RTC for MQTT timestamp
        time_ns = None
        if NTP_SERVER:
            pool = socketpool.SocketPool(my_wifi.wifi.radio)
            time_ns = my_ntp.time_ns(pool, NTP_SERVER)
        if MQTT_PUB:
            my_mqtt.mqtt_connect(mqtt_client)
            mqtt_client.loop()
        # Get probe value and publish to MQTT
        for pin in probes:
            bits = probes[pin]["analog_in"].value
            wet = wet_pct(pin, SAMPLES)
            print(f"{pin}: {bits} bits, wet: {wet:.1f}%")
            if MQTT_PUB:
                publish_influx(pin, wet, mqtt_client, time_ns)
        if MQTT_PUB:
            my_mqtt.mqtt_disconnect(mqtt_client)
        time.sleep(QUERY_INT)


if __name__ == "__main__":
    main()
