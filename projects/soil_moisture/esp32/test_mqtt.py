import my_mqtt

mqtt_client = my_mqtt.get_client()
my_mqtt.mqtt_connect(mqtt_client)
print("client connected: %s" % my_mqtt.mqtt_is_connected(mqtt_client))
print("broker: %s" % mqtt_client.broker)
my_mqtt.mqtt_publish('test/esp32/hi', 'hello', mqtt_client)
my_mqtt.mqtt_disconnect(mqtt_client)
