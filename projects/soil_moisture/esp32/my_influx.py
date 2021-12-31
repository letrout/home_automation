"""
Helper code for fomratting messages destined for InfluxDB2

FIXME: This is a copy of sensors/lib/influx/influx.py
Need to re-arrange this code to get all CircuitPython libs in one place
"""

import my_ntp

# These are treated as boolean by influx LP
BOOLS = ["t", "T", "true", "True", "TRUE", "f", "F", "false", "False", "FALSE"]


def influx_lp(measurement, fields, tags=None, time_ns=None, get_time=False):
    """
    Build InfluxDB input in line protocol syntax
    ref: https://docs.influxdata.com/influxdb/v2.0/reference/syntax/line-protocol/
        Parameters
            measurement (str): the measurement name (rewuired)
            fields (dict): key->value of LP field set (required)
            tags (dict): key->value of LP tag set (optional)
            time_ns (int): timestamp in nsec (optional)
            get_time (bool): if True (and time_ns None) try to get time
        Returns
            lp (str): InfluxDB input in line protocol syntax (or None on error)
    """
    msg = None
    if time_ns is None and get_time:
        time_ns = my_ntp.time_ns()
    # validate input
    if (
        (not measurement)
        or (not isinstance(measurement, str))
        or (not isinstance(fields, dict))
        or (len(list(fields.keys())) == 0)
        or (tags and not isinstance(tags, dict))
    ):
        return None
    ts = lp_set(tags)
    fs = lp_set(fields)
    if ts is None:
        msg = f"{str(measurement)} {fs}"
    else:
        msg = f"{str(measurement)},{ts} {fs}"
    if time_ns is not None:
        msg = msg + f" {time_ns}"
    return msg


def lp_set(fields):
    """
    create a line protocol fieldset string from a dict
        Parameters
            fields (dict): dictionary of field->value
        Returns
            lp_fieldset (str): string of "field1=value1,field2=value2..."
            None on error or empty
    FIXME: how do we handle spaces in strings? Embedding quotes in field keys
    or values results in weird behavior in inflixdb queries
    """
    if not isinstance(fields, dict) or len(list(fields.keys())) == 0:
        return None
    fieldset = []
    lp_fieldset = ""
    for key in fields:
        val = fields[key]
        if isinstance(val, str):
            # FIXME: handle spaces in strings
            if val not in BOOLS:
                val = f"{val}"
        fieldset.append(f"{key}={val}")
    if len(fieldset) > 0:
        lp_fieldset += ",".join(fieldset)
    return lp_fieldset
