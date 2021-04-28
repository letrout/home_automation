# MQTT configuration settings
# Modify according to this hosts's specific environment
# Assumptions:
#   Each host will use a single MQTT broker for all events
#   Each host can have multiple sensors (indexed by 'pin'),
#       with an event bucket destination defined for each sensor

MQTT_BROKER = '127.0.0.1'
MQTT_PORT = 1883
MQTT_USER = 'ha'
MQTT_PW = 'password'
PINS = {
    4: {
        'topic': 'mbr/door'
    }
}