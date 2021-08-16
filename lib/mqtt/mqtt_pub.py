# Code to publish event to MQTT broker

from datetime import datetime
import paho.mqtt.client as paho

# Get MQTT details from our config file
from . import mqtt_conf

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
    client1 = paho.Client("control1")
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

def publish(pin, dest, val, client1):
    topic = mqtt_conf.PINS[pin]['topic'] + '/' + dest
    ret = client1.publish(topic, val)
    return ret

def disconnect(mqtt_client):
    mqtt_client.disconnect()
