# SPDX-FileCopyrightText: 2017 Scott Shawcroft, written for Adafruit Industries
# SPDX-FileCopyrightText: Copyright (c) 2021 Melissa LeBlanc-Williams
# for Adafruit Industries
#
# SPDX-License-Identifier: MIT
import time
import json
from fh import MyFunHouse, TEMP_LABEL, HUMIDITY_LABEL, PRESSURE_LABEL

PUBLISH_DELAY = 60
ENVIRONMENT_CHECK_DELAY = 5
ENABLE_PIR = True
MQTT_TOPIC = "funhouse/state"
LIGHT_STATE_TOPIC = "funhouse/light/state"
LIGHT_COMMAND_TOPIC = "funhouse/light/set"
INITIAL_LIGHT_COLOR = 0x008000


def loop(
        fh, last_peripheral_state, last_environment_timestamp,
        last_publish_timestamp):
    while True:
        if (len(fh.environment) == 0) or (
            time.monotonic() - last_environment_timestamp > ENVIRONMENT_CHECK_DELAY
        ):
            fh.update_enviro()
            last_environment_timestamp = time.monotonic()
        output = fh.environment

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

    fh = MyFunHouse(
        temp="aht20", hum="aht20", press="dp310",
        topic_state=MQTT_TOPIC, topic_lc=LIGHT_COMMAND_TOPIC, topic_ls=LIGHT_STATE_TOPIC
        )

    # Add the labels
    fh.funhouse.add_text(
            text="Temperature:",
            text_position=(20, 30),
            text_color=0xFF8888,
            text_font="fonts/Arial-Bold-24.pcf",
        )
    fh.set_label(
        TEMP_LABEL,
        text_position=(120, 60),
        text_anchor_point=(0.5, 0.5),
        text_color=0xFFFF00,
        text_font="fonts/Arial-Bold-24.pcf",
    )
    fh.funhouse.add_text(
        text="Humidity:",
        text_position=(20, 100),
        text_color=0x8888FF,
        text_font="fonts/Arial-Bold-24.pcf",
    )
    fh.set_label(
        HUMIDITY_LABEL,
        text_position=(120, 130),
        text_anchor_point=(0.5, 0.5),
        text_color=0xFFFF00,
        text_font="fonts/Arial-Bold-24.pcf",
    )
    fh.funhouse.add_text(
        text="Pressure:",
        text_position=(20, 170),
        text_color=0xFF88FF,
        text_font="fonts/Arial-Bold-24.pcf",
    )
    fh.set_label(
        PRESSURE_LABEL,
        text_position=(120, 200),
        text_anchor_point=(0.5, 0.5),
        text_color=0xFFFF00,
        text_font="fonts/Arial-Bold-24.pcf",
    )

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

    fh.update_enviro()
    last_environment_timestamp = time.monotonic()

    # Provide Initial light state
    fh.publish_light_state()

    loop(
        fh, last_peripheral_state, last_environment_timestamp,
        last_publish_timestamp)


if __name__ == "__main__":
    main()
