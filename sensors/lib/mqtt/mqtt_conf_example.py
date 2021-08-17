# MQTT configuration settings
# Modify according to this hosts's specific environment
# Assumptions:
#   Each host will use a single MQTT broker for all events
#   Each host can have multiple sensors (indexed by 'pin'),
#       with an event bucket destination defined for each sensor

MQTT_BROKER = '<MQTT broker IP>'
MQTT_PORT = <MQTT broker port>
MQTT_USER = '<username>'
MQTT_PW = '<password>'
PINS = {
    <pin#>: {
        'topic': '<sub1>/<sub2>'
    }
    <pin#>: {
        'topic': '<sub1>/<sub3>'
    }
}
