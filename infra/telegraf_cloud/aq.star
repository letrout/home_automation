# Filter airnow.org AQI metrics
'''
AirNow is reporting "AQI: -1" for forecast entries, which is 
preventing me from ingesting any valid values (eveything in influx is -1)
If the value is -1, we delete all the fields, effectively dropping the metric.

'''

def apply(metric):
    if metric.fields["AQI"] == -1:
        # removing all fields deletes a metric
        # TODO: drop the metric? just drop AQI?
        metric.fields.clear()
    return metric
