# Code to publish event to MQTT broker

from datetime import datetime
import paho.mqtt.client as paho

# Get MQTT details from our config file
from . import mqtt_conf

# publish callabck function
def on_publish(client, userdata, result):
    print(f'{datetime.now()} - data published')
    pass

# disconnect callback
def on_disconnect(client, userdata, rc):
   print(f'{datetime.now()} - client disconnected')
   pass

def mqtt_client():
    client1 = paho.Client("control1")
    client1.on_publish = on_publish
    client1.on_disconnect = on_disconnect
    return client1

def mqtt_pulish(mqtt_client, dest, val):
    mqtt_client.connect(mqtt_conf.MQTT_BROKER, mqtt_conf.MQTT_PORT)
    ret = mqtt_client.publish(dest, val)
    return ret

def mqtt_disconnect(mqtt_client):
    mqtt_client.disconnect()
