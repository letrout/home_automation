# Code to publish event to MQTT broker

from datetime import datetime
import paho.mqtt.client as paho

# MQTT details
MQTT_BROKER="127.0.0.1"
MQTT_PORT=1883

# publish callabck function
def on_publish(client,userdata,result):
    print("data published \n")
    pass

# disconnect callback
def on_disconnect(client, userdata, rc):
   print("client disconnected ok")

def mqtt_client():
    client1= paho.Client("control1") 
    client1.on_publish = on_publish
    client1.on_disconnect = on_disconnect
    return client1

def mqtt_pulish(mqtt_client, dest, val):
    mqtt_client.connect(MQTT_BROKER,MQTT_PORT)
    ret = mqtt_client.publish(dest, val)
    return ret

def mqtt_disconnect(mqtt_client):
    mqtt_client.disconnect()
