#
# Based on https://learn.adafruit.com/mqtt-in-circuitpython/connecting-to-a-mqtt-broker
#
# FIXME: need exception handling on client calls?
# TODO: influx LP formatting

import adafruit_minimqtt.adafruit_minimqtt as MQTT
import socketpool
import ssl

import my_wifi
# Get MQTT details and more from a secrets.py file
try:
    from secrets import secrets
except ImportError:
    print("MQTT secrets are kept in secrets.py, please add them there!")
    raise

# Default topic
mqtt_topic = "test/topic"


# Define callback methods which are called when events occur
# pylint: disable=unused-argument, redefined-outer-name
def connect(mqtt_client, userdata, flags, rc):
    # This function will be called when the mqtt_client is connected
    # successfully to the broker.
    print("Connected to MQTT Broker!")
    print("Flags: {0}\n RC: {1}".format(flags, rc))


def disconnect(mqtt_client, userdata, rc):
    # This method is called when the mqtt_client disconnects
    # from the broker.
    print("Disconnected from MQTT Broker!")


def subscribe(mqtt_client, userdata, topic, granted_qos):
    # This method is called when the mqtt_client subscribes to a new feed.
    print("Subscribed to {0} with QOS level {1}".format(topic, granted_qos))


def unsubscribe(mqtt_client, userdata, topic, pid):
    # This method is called when the mqtt_client unsubscribes from a feed.
    print("Unsubscribed from {0} with PID {1}".format(topic, pid))


def publish(mqtt_client, userdata, topic, pid):
    # This method is called when the mqtt_client publishes data to a feed.
    print("Published to {0} with PID {1}".format(topic, pid))


def message(client, topic, message):
    # Method callled when a client's subscribed feed has a new value.
    print("New message on topic {0}: {1}".format(topic, message))


def get_client():
    if my_wifi.wifi.radio.ipv4_address is None:
        my_wifi.connect()
        if my_wifi.wifi.radio.ipv4_address is None:
            print("ERROR: mqtt client failed to connect to WiFi")
            return None


    mqtt_client = MQTT.MQTT(
        broker=secrets["mqtt_broker"],
        port=secrets["mqtt_port"],
        username=secrets["mqtt_user"],
        password=secrets["mqtt_password"],
        socket_pool=socketpool.SocketPool(my_wifi.wifi.radio),
        ssl_context=ssl.create_default_context()
        )
    # Connect callback handlers to mqtt_client
    mqtt_client.on_connect = connect
    mqtt_client.on_disconnect = disconnect
    mqtt_client.on_subscribe = subscribe
    mqtt_client.on_unsubscribe = unsubscribe
    mqtt_client.on_publish = publish
    mqtt_client.on_message = message
    return mqtt_client

def mqtt_connect(client=None):
    if client is None:
        client = get_client()
    if client is not None:
        print("Attempting to connect to %s" % client.broker)
        client.connect()
    return client


def mqtt_subscribe(topic, client=None):
    if client is None:
        client = get_client()
    if client is not None:
        print("Subscribing to %s" % topic)
        client.subscribe(mqtt_topic)
    return client


def mqtt_publish(topic, client=None):
    if client is None:
        client = get_client()
    if client is not None:
        print("Publishing to %s" % topic)
        client.publish(topic, "Hello Broker!")
    return client


def mqtt_unsubscribe(topic, client=None):
    if client is None:
        client = get_client()
    if client is not None:
        print("Unsubscribing from %s" % topic)
        client.unsubscribe(topic)
    return client


def mqtt_disconnect(client):
    if client is not None:
        print("Disconnecting from %s" % client.broker)
        client.disconnect()
