# influxdb2
influxdb2 server setup
# Make directories for persistent volumes
	$ sudo mkdir /docker/influxdb2/data
	$ sudo mkdir /docker/influxdb2/config
# Run InfluxDB with docker-compose
	$ sudo docker-compose up -d
## Configure the influxdb server with data from old server
	Get the token from the old server
	$ sudo docker exec influxdb2 influx auth list --user joel --json | jq -r '.[].token'
	Shutdown the telegraf server
	Create a backup on the old server
	$ sudo docker exec influxdb2 influx backup /var/lib/influxdb2/backup_<date>
	Copy the backup to /docker/influxdb2/data/ on the new server
	$ sudo docker exec -it influxdb2 bash
	# influx setup --token <admin token from old server>
	# influx restore /var/lib/influxdb2/<backup dir> --full
	Verify data
	Change 'influxdb2' CNAME to point to new server
	Restart telegraf

# Run InfluxDB 2.0 with Docker (this was used to create my original InfluxDB server)
	$ docker network create influxdb2
	$ sudo docker run -d -p 8086:8086 \
	      --restart unless-stopped \
	      --name=influxdb2 \
	      --net=influxdb2 \
	      -v /docker/influxdb2/data:/var/lib/influxdb2 \
	      -v /docker/influxdb2/config:/etc/influxdb2 \
	      -e DOCKER_INFLUXDB_INIT_MODE=setup \
	      -e DOCKER_INFLUXDB_INIT_USERNAME=$INFLUX_USER \
	      -e DOCKER_INFLUXDB_INIT_PASSWORD=$INFLUX_PW \
	      -e DOCKER_INFLUXDB_INIT_ORG=$INFLUX_ORG \
	      -e DOCKER_INFLUXDB_INIT_BUCKET=$INFLUX_BUCKET \
	      influxdb:2.0-alpine

# Set up Telegraf
	- https://docs.influxdata.com/influxdb/v2.0/write-data/no-code/use-telegraf/auto-config/
	- Under influxdb tokens there should be one named WRITE <bucket> bucket / READ <bucket> telegraf config, copy it
	- In the influxdb2 container, run
		export INFLUX_TOKEN=<influxdb2 token>
		telegraf -config http://localhost:8086/api/v2/telegrafs/0xoX00oOx0xoX00o

# Set up MQTT subscription(s)
