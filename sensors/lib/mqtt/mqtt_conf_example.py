# MQTT configuration settings
# Modify according to this hosts's specific environment
# and rename to 'mqtt_conf.py'
# Assumptions:
#   Each host will use a single MQTT broker for all events
#   Each host can have multiple sensors (indexed by 'pin'),
#       with an event bucket destination defined for each sensor

MQTT_BROKER = '<MQTT broker IP>'
MQTT_PORT = <MQTT broker port>
MQTT_USER = '<username>'
MQTT_PW = '<password>'
INFLUX_PREFIX = 'influx/'   # prepend influx-destined topics with this
PINS = {
    <pin1>: {
        'location': '<location>',
        'room': '<room>',
        'room_loc': '<room_loc>',
        'topic': '<sub1>/<sub2>'
    }
    <pin2>: {
        'location': '<location>',
        'room': '<room>',
        'room_loc': '<room_loc>',
        'topic': '<sub1>/<sub3>'
    }
}
