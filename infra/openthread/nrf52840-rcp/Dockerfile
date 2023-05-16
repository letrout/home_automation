#FROM debian:11-slim
FROM ubuntu:23.04

RUN apt-get update \
	&& apt-get install -y --no-install-recommends \
                ca-certificates \
                git \
                sudo \
	&& apt-get clean \
	&& apt-get autoremove -y

# Get the source
RUN git clone https://github.com/openthread/ot-nrf528xx.git
WORKDIR ./ot-nrf528xx
RUN git submodule update --init

# Install the ARM toolchain
RUN ./script/bootstrap

# Build the flash image nRF52840 dongle support (PCA10059)
RUN ./script/build nrf52840 USB_trans -DOT_BOOTLOADER=USB

# Convert to hex
RUN arm-none-eabi-objcopy -O ihex build/bin/ot-cli-ftd ot-cli-ftd.hex
