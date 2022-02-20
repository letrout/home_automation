# SPDX-FileCopyrightText: 2017 Scott Shawcroft, written for Adafruit Industries
# SPDX-FileCopyrightText: Copyright (c) 2021 Melissa LeBlanc-Williams for Adafruit Industries
#
# SPDX-License-Identifier: MIT
import time
import json
from fh import MyFunHouse

PUBLISH_DELAY = 60
ENVIRONMENT_CHECK_DELAY = 5
ENABLE_PIR = True
MQTT_TOPIC = "funhouse/state"
LIGHT_STATE_TOPIC = "funhouse/light/state"
LIGHT_COMMAND_TOPIC = "funhouse/light/set"
INITIAL_LIGHT_COLOR = 0x008000
USE_FAHRENHEIT = True

try:
    from secrets import secrets
except ImportError:
    print("WiFi secrets are kept in secrets.py, please add them there!")
    raise


def update_enviro(fh, environment):

    temp = fh.funhouse.peripherals.temperature
    unit = "C"
    if USE_FAHRENHEIT:
        temp = temp * (9 / 5) + 32
        unit = "F"

    environment["temperature"] = temp
    environment["pressure"] = fh.funhouse.peripherals.pressure
    environment["humidity"] = fh.funhouse.peripherals.relative_humidity
    environment["light"] = fh.funhouse.peripherals.light

    fh.funhouse.set_text("{:.1f}{}".format(environment["temperature"], unit), fh.display.temp_label)
    fh.funhouse.set_text("{:.1f}%".format(environment["humidity"]), fh.display.hum_label)
    fh.funhouse.set_text("{}hPa".format(environment["pressure"]), fh.display.pres_label)


def connected(client, userdata, result, payload):
    # FIXME: how to access fh status
    #status.fill = 0x00FF00
    #status.outline = 0x008800
    print("Connected to MQTT! Subscribing...")
    client.subscribe(LIGHT_COMMAND_TOPIC)


def disconnected(client):
    # FIXME: how to access fh status
    #status.fill = 0xFF0000
    #status.outline = 0x880000
    pass


def message(client, topic, payload, funhouse):
    print("Topic {0} received new value: {1}".format(topic, payload))
    if topic == LIGHT_COMMAND_TOPIC:
        settings = json.loads(payload)
        if settings["state"] == "on":
            if "brightness" in settings:
                funhouse.peripherals.dotstars.brightness = settings["brightness"] / 255
            else:
                funhouse.peripherals.dotstars.brightness = 0.3
            if "color" in settings:
                funhouse.peripherals.dotstars.fill(settings["color"])
        else:
            funhouse.peripherals.dotstars.brightness = 0
        publish_light_state()


def publish_light_state(fh):
    fh.funhouse.peripherals.led = True
    output = {
        "brightness": round(fh.funhouse.peripherals.dotstars.brightness * 255),
        "state": "on" if fh.funhouse.peripherals.dotstars.brightness > 0 else "off",
        "color": fh.funhouse.peripherals.dotstars[0],
    }
    # Publish the Dotstar State
    print("Publishing to {}".format(LIGHT_STATE_TOPIC))
    fh.funhouse.network.mqtt_publish(LIGHT_STATE_TOPIC, json.dumps(output))
    fh.funhouse.peripherals.led = False


def loop(
        fh, environment, last_peripheral_state, last_environment_timestamp,
        last_publish_timestamp):
    while True:
        if not environment or (
            time.monotonic() - last_environment_timestamp > ENVIRONMENT_CHECK_DELAY
        ):
            update_enviro(fh, environment)
            last_environment_timestamp = time.monotonic()
        output = environment

        peripheral_state_changed = False
        for peripheral in last_peripheral_state:
            current_item_state = getattr(fh.funhouse.peripherals, peripheral)
            output[peripheral] = "on" if current_item_state else "off"
            if last_peripheral_state[peripheral] != current_item_state:
                peripheral_state_changed = True
                last_peripheral_state[peripheral] = current_item_state

        if fh.funhouse.peripherals.slider is not None:
            output["slider"] = fh.funhouse.peripherals.slider
            peripheral_state_changed = True

        # Every PUBLISH_DELAY, write temp/hum/press/light or if a peripheral changed
        if (
            last_publish_timestamp is None
            or peripheral_state_changed
            or (time.monotonic() - last_publish_timestamp) > PUBLISH_DELAY
        ):
            fh.funhouse.peripherals.led = True
            print("Publishing to {}".format(MQTT_TOPIC))
            fh.funhouse.network.mqtt_publish(MQTT_TOPIC, json.dumps(output))
            fh.funhouse.peripherals.led = False
            last_publish_timestamp = time.monotonic()

        # Check any topics we are subscribed to
        fh.funhouse.network.mqtt_loop(0.5)


def main():
    global status

    fh = MyFunHouse()

    # Initialize a new MQTT Client object
    fh.funhouse.network.init_mqtt(
        secrets["mqtt_broker"],
        secrets["mqtt_port"],
        secrets["mqtt_user"],
        secrets["mqtt_password"],
    )
    fh.funhouse.network.on_mqtt_connect = connected
    fh.funhouse.network.on_mqtt_disconnect = disconnected
    fh.funhouse.network.on_mqtt_message = message

    print("Attempting to connect to {}".format(secrets["mqtt_broker"]))
    fh.funhouse.network.mqtt_connect()

    last_publish_timestamp = None

    last_peripheral_state = {
        "button_up": fh.funhouse.peripherals.button_up,
        "button_down": fh.funhouse.peripherals.button_down,
        "button_sel": fh.funhouse.peripherals.button_sel,
        "captouch6": fh.funhouse.peripherals.captouch6,
        "captouch7": fh.funhouse.peripherals.captouch7,
        "captouch8": fh.funhouse.peripherals.captouch8,
    }

    if ENABLE_PIR:
        last_peripheral_state["pir_sensor"] = fh.funhouse.peripherals.pir_sensor

    environment = {}
    update_enviro(fh, environment)
    last_environment_timestamp = time.monotonic()

    # Provide Initial light state
    publish_light_state(fh)

    loop(
        fh, environment, last_peripheral_state, last_environment_timestamp,
        last_publish_timestamp)


if __name__ == "__main__":
    main()
