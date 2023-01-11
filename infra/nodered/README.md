# Node-RED with docker-compose
## set up  directories for persistent volumes
	$ mkdir -p /docker/nodered/data
	$ chown 1000.1000 /docker/nodered/data
## run the container
	$ sudo docker-compose up -d
## Set up HomeAssistant
### Set up Node-RED for HA
	In the Node-RED pallet manager, install
	node-red-contrib-home-assistant-websocket
### Set up in HA
	HACS->Integrations->Node-RED Companion install
	Restart HA
	Refresh browser
	Settings->Integrations->Node-RED Companion
		Submit
### Set up HA in Node-RED
	TBD
