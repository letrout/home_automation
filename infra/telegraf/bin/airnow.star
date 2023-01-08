# Filter airnow.org AQI metrics
'''
Convert the time fields from AirNow current observation result
'''

load('time.star', 'time')

def apply(metric):
    # TODO: use the "LocalTimeZone" field
#    metric.time = metric.time + (int(metric.fields['HourObserved']) * 3600 * 1e9)
    #metric.time = metric.fields['HourObserved'] * 3600 * 1e9
    metric.time = time.parse_time("{} {}:00:00".format(metric.fields['DateObserved'].strip(), int(metric.fields['HourObserved'])), format="2006-01-02 15:04:05", location="US/Central").unix_nano
    metric.fields.pop('HourObserved')
    metric.fields.pop('DateObserved')
    return metric
