# Code to publish event to MQTT broker

from datetime import datetime
import paho.mqtt.client as paho
import socket

# Get MQTT details from our config file
from . import mqtt_conf
from lib.influx import influx

# Connection callback
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("connected OK Returned code=", rc)
    else:
        print("Bad connection Returned code=", rc)

# publish callabck function
def on_publish(client, userdata, result):
    print(f'{datetime.now()} - data published')
    pass

# disconnect callback
def on_disconnect(client, userdata, rc):
   print(f'{datetime.now()} - client disconnected')
   pass

def client():
    client1 = paho.Client(socket.gethostname())
    client1.username_pw_set(
        username=mqtt_conf.MQTT_USER, password=mqtt_conf.MQTT_PW)
    client1.on_connect = on_connect
    client1.on_publish = on_publish
    client1.on_disconnect = on_disconnect
    client1.connect(
        mqtt_conf.MQTT_BROKER,
        port=mqtt_conf.MQTT_PORT,
        keepalive=60,
        bind_address='')

    return client1

def pin_topic(pin):
    """
    construct a generic topic for one of our snesors (on a pin)
    """
    return (
        f'{mqtt_conf.PINS[pin]["location"]}/sensors/{mqtt_conf.PINS[pin]["room"]}'
        f'/{mqtt_conf.PINS[pin]["room_loc"]}'
        )

def influx_topic(pin):
    """
    construct an influxdb topic for one of our snesors (on a pin)
    influxdb will typically listen on specific topics, so we can
    publish in influx-specific syntax (line protocol) to those topics
    """
    return f"{mqtt_conf.INFLUX_PREFIX}{pin_topic(pin)}"

def publish(pin, dest, val, client1):
    """
    generic publish for a single measurement
    """
    topic = f"{pin_topic(pin)}/{dest}"
    ret = client1.publish(topic, val)
    return ret

def publish_influx(pin, client1, fields, tags=None, time_ns=None):
    """
    publish to an influx-specifc topic
    uses influxdb line protocol for data
    """
    measurement = "environment"
    topic = influx_topic(pin)
    if not isinstance(tags, dict) or len(tags.keys()) == 0:
        my_tags = {}
    else:
        my_tags = dict(tags)
    if "location" in mqtt_conf.PINS[pin].keys():
        my_tags["location"] = mqtt_conf.PINS[pin]["location"]
    if "room" in mqtt_conf.PINS[pin].keys():
        my_tags["room"] = mqtt_conf.PINS[pin]["room"]
    if "room_loc" in mqtt_conf.PINS[pin]:
        my_tags["room_loc"] = mqtt_conf.PINS[pin]["room_loc"]
    lp = influx.influx_lp(measurement, fields, my_tags, time_ns)
    if lp is None:
        return None
    else:
        return client1.publish(topic, lp)

def disconnect(mqtt_client):
    mqtt_client.disconnect()
