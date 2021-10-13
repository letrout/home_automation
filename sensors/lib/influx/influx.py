"""
Helper code for fomratting messages destined for InfluxDB2
"""

import time


def influx_lp(measurement, fields, tags=None, time_ns=None):
    """
    Build InfluxDB input in line protocol syntax
    ref: https://docs.influxdata.com/influxdb/v2.0/reference/syntax/line-protocol/
        Parameters
            measurement (str): the measurement name (rewuired)
            fields (dict): key->value of LP field set (required)
            tags (dict): key->value of LP tag set (optional)
            time_ns (int): timestamp in nsec (optional)
        Returns
            lp (str): InfluxDB input in line protocol syntax (or None on error)
    """
    if time_ns is None:
        time_ns = time.time_ns()
    # validate input
    if (
        (not measurement)
        or (not isinstance(measurement, str))
        or (not isinstance(fields, dict))
        or (not isinstance(time_ns, int))
        or (tags and not isinstance(tags, dict))
    ):
        return None
    lp = f"{str(measurement)} "
    tagset  = []
    if tags:
        for key in tags:
            tagset.append(f"{key}={tags[key]}")
    if len(tagset) > 0:
        lp += ",".join(tagset)
        lp += " "
    fieldset = []
    if fields:
        for key in fields:
            fieldset.append(f"{key}={fields[key]}")
    if len(fieldset) > 0:
        lp += ",".join(fieldset)
        lp += " "
    lp += f"{time_ns}"
    return lp
