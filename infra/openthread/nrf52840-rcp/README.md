#nRF52840 OpenThread Border Router RCP


Builds firmware for nRF8240 dongle, for use as a Radio Co-processor for  
an OpenThread Border Router device.

## Usage
	$ sudo docker build -t nrf52840-rcp .
	$ sudo docker run --rm -v $PWD:/out nrf52840-rcp:latest cp ot-nrf528xx/ot-cli-ftd.hex /out/
The firmware is hex format will be in $PWD/ot-cli-ftd.hex

## References
OTBR project: https://openthread.io/guides/border-router
OTBR build with nRF52840: https://openthread.io/codelabs/openthread-hardware#0
nRF52840 firmware: https://github.com/openthread/ot-nrf528xx/tree/main/src/nrf52840
