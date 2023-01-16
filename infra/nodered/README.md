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
	Generate access token
		Click on user icon->generate long-lived access token
### Set up HA in Node-RED
	Place a home assstant node, double-click on it
		Edit the Server field
		Enter the Home Assistant URL, token
## Other flows
### bigtimer
	https://flows.nodered.org/node/node-red-contrib-bigtimer
### stoptimer
	https://flows.nodered.org/node/node-red-contrib-stoptimer
### weekday
	https://flows.nodered.org/node/node-red-contrib-weekday
