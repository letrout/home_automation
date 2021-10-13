import pytest

from sensors.lib.influx import influx


def test_lp_1f1v():
    test = influx.influx_lp(
        "temp",
        {"field1": 1},
        {"tag1": 2},
        1634158455045502066)
    assert test == "temp tag1=2 field1=1 1634158455045502066"
