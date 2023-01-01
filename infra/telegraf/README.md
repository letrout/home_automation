# telegraf with docker-compose
## set up  directories for persistent volumes
	$ mkdir -p /docker/telegraf/conf/telegraf.d
	$ mkdir /docker/telegraf/bin
## Edit the telegraf env variables
	$ cp env_file env_file_secret
	edit env_file_secret with appropriate values
## Copy the config files
	$ cp telegraf.d/* /docker/telegraf/conf/telegraf.d/
## Copy the utility scripts
	$ cp aq.star /docker/telegraf/bin/
	$ cp inputs/influxdb2_bucket/bucket_k.sh /docker/telegraf/bin/
	$ chmod +x /docker/telegraf/bin/*
## run the container
	$ sudo docker-compose up -d
# telegraf docker container (same host as influxdb container)
## Generate a config file
	$ docker run --rm telegraf telegraf config > /docker/telegraf/telegraf.conf

## Run influxdb2
	$ docker network create influxdb
	<start influxdb2 container>
	get the influxdb API token

## Edit the telegraf config for influxdb
[[outputs.influxdb_v2]]
  urls = ["http://influxdb:8086"]
  token = "<token>
  organization = "$DOCKER_INFLUXDB_INIT_ORG"
  bucket = "$DOCKER_INFLUXDB_INIT_BUCKET"

## Add sources to the telegraf config
[TBD]

## Add influxdb token(s) to telegraf.conf
	Search for "REPLACE_ME" in the config, replace with influxdb token(s)
	
## Run telegraf container
	docker run -d --name telegraf \
	--restart unless-stopped \
	--net=container:influxdb2 \
	-v /docker/telegraf/telegraf.conf:/etc/telegraf/telegraf.conf:ro \
	-v /var/run/docker.sock:/var/run/docker.sock:ro \
	-v /docker/influxdb2/data/bucket_du.txt:/influxdb2/bucket_du.txt:ro \
	-v /:/hostfs:ro \
	-e HOST_ETC=/hostfs/etc \
	-e HOST_PROC=/hostfs/proc \
	-e HOST_SYS=/hostfs/sys \
	-e HOST_VAR=/hostfs/var \
	-e HOST_RUN=/hostfs/run \
	-e HOST_MOUNT_PREFIX=/hostfs \
	-e DOCKER_INFLUXDB_INIT_ORG=$INFLUX_ORG \
	-e DOCKER_INFLUXDB_INIT_BUCKET=$INFLUX_BUCKET \
	telegraf

# Copy starlark script for preprocessing AirNow data
# TODO: build a docker image with the starlark script?
sudo docker cp aq.star telegraf:/usr/local/bin
sudo docker exec telegraf chmod +x /usr/local/bin/aq.star

# Copy shell script to collect bucket size data
# TODO: build a docker image with the bucket script?
sudo docker cp inputs/influxdb2_bucket/bucket_du.sh telegraf:/usr/local/bin
sudo docker exec telegraf chmod +x /usr/local/bin/bucket_du.sh
# Run the following on the host's crontab (assuming telegraf & influxdb2 are same host)
# run at same or more frequent than telegraf exec plugin interval
{ du -sk /docker/influxdb2/data/engine/data/* & du -sk /docker/influxdb2/data/engine/wal/*; } >/docker/influxdb2/data/bucket_du.txt 2>/dev/null

Test the telegraf container
	sudo docker exec telegraf telegraf --test

Telegraf on Pi
  Get the Debian version
	cat /etc/os-release
	PRETTY_NAME="Raspbian GNU/Linux 10 (buster)"
  Add the repo key
	curl -sL https://repos.influxdata.com/influxdb.key | sudo apt-key add -
	OK
  Add the repo
	echo "deb https://repos.influxdata.com/debian buster stable" | sudo tee /etc/apt/sources.list.d/influxdb.list
	deb https://repos.influxdata.com/debian buster stable
  Install telegraf
	sudo apt-get update
	sudo apt-get install telegraf
  Start telegraf
	update /etc/telegraf/telegraf.conf, /etc/telegraf/telegrad.d/*
	sudo systemctl reload telegraf.service
  Add the telegraf user to video group, so it can run vcgencmd (to get GPU temp)
	sudo usermod -a -G video telegraf
  Test the command
	sudo -u telegraf vcgencmd measure_temp

