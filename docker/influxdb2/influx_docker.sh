#!/bin/bash
docker run -d --name influxdb2 -p 8086:8086 \
      --net=influxdb2 \
      -v /docker/influxdb2/data:/var/lib/influxdb2 \
      -v /docker/influxdb2/config:/etc/influxdb2 \
      -e DOCKER_INFLUXDB_INIT_MODE=setup \
      -e DOCKER_INFLUXDB_INIT_USERNAME=$INFLUX_USER \
      -e DOCKER_INFLUXDB_INIT_PASSWORD=$INFLUX_PW \
      -e DOCKER_INFLUXDB_INIT_ORG=$INFLUX_ORG \
      -e DOCKER_INFLUXDB_INIT_BUCKET=$INFLUX_BUCKET \
      influxdb:2.0-alpine
