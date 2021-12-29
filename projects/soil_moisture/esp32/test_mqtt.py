import my_mqtt

mqtt_client = my_mqtt.get_client()
my_mqtt.mqtt_connect(mqtt_client)
print("client connected; %s" % mqtt_client.is_connected())
print("broker: %s" % mqtt_client.broker)
my_mqtt.mqtt_publish('test/esp32/hi', 'hello', mqtt_client)
my_mqtt.mqtt_disconnect(mqtt_client)
