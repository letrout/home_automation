import pytest

from sensors.lib.influx import influx


def test_lp_1f1v():
    test = influx.influx_lp(
        "temp",
        {"field1": 1},
        {"tag1": 2},
        1634158455045502066
        )
    assert test == "temp tag1=2 field1=1 1634158455045502066"


def test_lp_2f2v():
    test = influx.influx_lp(
        "temp",
        {"field1": 1, "field2": 5.5},
        {"tag1": 2, "tag2": 6},
        1634158455045502066
        )
    assert test == "temp tag1=2,tag2=6 field1=1,field2=5.5 1634158455045502066"


def test_lp_string():
    test = influx.influx_lp(
        "temp",
        {"field1": 1, "field2": "a string"},
        {"tag1": 2, "tag2": 6},
        1634158455045502066
        )
    assert test == """temp tag1=2,tag2=6 field1=1,field2="a string" 1634158455045502066"""


def test_lp_bad_ts():
    test = influx.influx_lp(
        "temp",
        {"field1": 1},
        {"tag1": 2},
        "manynanoseconds"
        )
    assert test is None


def test_lp_bad_field():
    test = influx.influx_lp(
        "temp",
        ["field1", 1],
        {"tag1": 2},
        1634158455045502066
        )
    assert test is None


def test_lp_bad_tag():
    test = influx.influx_lp(
        "temp",
        {"field1": 1},
        "tag1",
        1634158455045502066
        )
    assert test is None
