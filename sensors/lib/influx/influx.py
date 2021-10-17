"""
Helper code for fomratting messages destined for InfluxDB2
"""

import time

# These are treated as boolean by influx LP
BOOLS = ["t", "T", "true", "True", "TRUE", "f", "F", "false", "False", "FALSE"]


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
        or (len(fields.keys()) == 0)
        or (not isinstance(time_ns, int))
        or (tags and not isinstance(tags, dict))
    ):
        return None
    ts = lp_set(tags)
    fs = lp_set(fields)
    if ts is None:
        return f"{str(measurement)} {fs} {time_ns}"
    else:
        return f"{str(measurement)},{ts} {fs} {time_ns}"


def lp_set(fields):
    """
    create a line protocol fieldset string from a dict
        Parameters
            fields (dict): dictionary of field->value
        Returns
            lp_fieldset (str): string of "field1=value1,field2=value2..."
            None on error or empty
    """
    if not isinstance(fields, dict) or len(fields.keys()) == 0:
        return None
    fieldset = []
    lp_fieldset = ""
    for key in fields:
        val = fields[key]
        if isinstance(val, str):
            if not val in BOOLS:
                val = f"\"{val}\""
        fieldset.append(f"{key}={val}")
    if len(fieldset) > 0:
        lp_fieldset += ",".join(fieldset)
    return lp_fieldset
