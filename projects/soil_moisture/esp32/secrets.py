secrets = {
    'ssid' : 'SSID',
    'password' : 'WIFI_PW',
    'mqtt_broker' : 'MQTT_BROKER',
    'mqtt_user' : 'MQTT_USER',
    'mqtt_password' : 'MQTT_PASSORD',
    'mqtt_port' : 'MQTT_PORT',
    'mqtt_topic' : 'influx/Owens/plants/',
    'aio_username' : 'my_adafruit_io_username',
    'aio_key' : 'my_adafruit_io_key',
    'ntp_server' : 'NTP_SERVER_IP',
    'timezone' : "America/Chicago", # http://worldtimeapi.org/timezones
    }

"""
inlfuxdb tags and measurement
"""
influx = {
    '_measurement': 'soil_moisture',
    'tags': {
        'location': 'Owens',
        'room': 'office',
        'room_loc': 'window'
    }
}

"""
probes aren't secrets per se, but they are part of the local config data
dry and wet values are empirically determined for each individual soil probe
dry: ADC value when probe in air
wet: ADC value when probe submerged in water
"""
probes = {
    "A1": {
        "analog_in": None,
        "bits_dry": 52586,
        "bits_wet": 29113,
        "plant": "pepper1"
    },
    "A2": {
        "analog_in": None,
        "bits_dry": 52586,
        "bits_wet": 28954,
        "plant": "pepper2"
    },
    "A3": {
        "analog_in": None,
        "bits_dry": 52586,
        "bits_wet": 28636,
        "plant": "pepper3"
    },
    "A4": {
        "analog_in": None,
        "bits_dry": 52586,
        "bits_wet": 28775,
        "plant": "pepper4"
    }
}
