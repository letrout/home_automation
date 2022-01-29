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


def read_pin(pin):
    """
    Read analog pin value
    params:
        pin - the pin number (key of dict in secrets)
    returns:
        bits - probe bits on success, None on failure
    """
    bits = None
    try:
        bits = probes[pin]["analog_in"].value
    except (ValueError, KeyError):
        pass
    return bits


def get_voltage(pin):
    """
    Get voltage value from an ADV pin
    params:
        pin - the pin number (key of dict in secrets)
    returns:
        volts - volts value on success, None on failure
    """
    bits = read_pin(pin)
    if bits is not None:
        return (probes[pin]["analog_in"].value * V_REF) / ((1 << ADC_BITS) - 1)
    else:
        return None


def dry_pct(pin, samples=1):
    """
    The percentage of "dry" into the probe's range
    params:
        pin - the pin number (key of dict in secrets)
        samples - the number of samples to take and average
    returns:
        dry_pct: the percentage of dry of the probe range, None on failure
    """
    bits = 0
    count = 0
    for i in range(0,samples):
        reading = read_pin(pin)
        if reading is not None:
            bits += reading
            count += 1
    if count > 0:
        bits /= count
        dry_pct = interp(
            bits,
            [probes[pin]["bits_wet"], probes[pin]["bits_dry"]],
            [0, 100]
            )[0]
    else:
        dry_pct = None
    return dry_pct


def wet_pct(pin, samples=1):
    """
    The percentage of "wet" into the probe's range
    params:
        pin - the pin number (key of dict in secrets)
        samples - the number of samples to take and average
    returns:
        wet_pct: the percentage of wet of the probe range, None on failure
    """
    dry = dry_pct(pin, samples)
    if dry is not None:
        return 100.0 - dry_pct(pin, samples)
    else:
        return None


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
            bits = read_pin(pin)
            wet = wet_pct(pin, SAMPLES)
            print(f"{pin}: {bits} bits, wet: {wet:.1f}%")
            if MQTT_PUB and (wet is not None):
                publish_influx(pin, wet, mqtt_client, time_ns)
        if MQTT_PUB:
            my_mqtt.mqtt_disconnect(mqtt_client)
        time.sleep(QUERY_INT)


if __name__ == "__main__":
    main()
