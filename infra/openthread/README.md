# openThread Border Router
See: https://openthread.io/guides/border-router

## Proxmox

### Load iptables
On the Proxmox host
    # modprobe ip6table_filter
### LXC USB passthrough
From: https://gist.github.com/crundberg/a77b22de856e92a7e14c81f40e7a74bd

If running in LXC in Proxmox, you will need to set up USB passthrough
for the RCP.

On the Proxmox host, find the device:
    root@dell-15z:~# lsusb |grep Nordic
    Bus 003 Device 002: ID 1915:521f Nordic Semiconductor ASA Open DFU Bootloader
Note the 'Bus' and 'Device' numbers (003 and 002 respectively in this example).

On the Proxmox host, get the device major and minor numbers from /dev/bus/usb/<bus>/<device>:
    root@dell-15z:~# ls -al /dev/bus/usb/003/002
    crw-rw-r-- 1 root root 189, 257 May 19 22:14 /dev/bus/usb/003/002
So we have 189 and 257 for major and minor, respectively.

On the Proxmox host, create a new device file with correct permissions:
    root@dell-15z:~# mkdir -p /lxc/101/devices
    root@dell-15z:~# cd /lxc/101/devices/
    root@dell-15z:/lxc/101/devices# mknod -m 660 ttyACM0 c 189 0
    root@dell-15z:/lxc/101/devices# chown 100000:100020 ttyACM0
    root@dell-15z:/lxc/101/devices# ls -al /lxc/101/devices/ttyACM0
    crw-rw---- 1 100000 100020 189, 0 May 20 15:41 /lxc/101/devices/ttyACM0

On the Proxmox host, edit the container config in /etc/pve/lxc/<container_id>.conf
    lxc.cgroup2.devices.allow: c 189:* rwm
    lxc.mount.entry: /lxc/101/devices/ttyACM0 dev/ttyACM0 none bind,optional,create=file

On the Proxmox host, edit the udev 
    root@dell-15z:~# vi /etc/udev/rules.d/50-myusb.rules
    SUBSYSTEM=="tty", ATTRS{idVendor}=="1915", ATTRS{idProduct}=="521f", MODE="0666", SYMLINK+="nrf52840"
    udevadm control --reload-rules && service udev restart && udevadm trigger

Restart the container, and look for RCP tty in the container to see if the device is visible.
    joel@docker-15z:~$ ls -l /dev/ttyACM*
    crw-rw---- 1 root dialout 189, 0 May 20 20:41 /dev/ttyACM0

## Flash the RCP
Build the RCP firmware per nrf52840-rcp

Note - I was unable to get this to work in a LXC, J-Link would not install
To flash the firmare, install NRF command line tools
    https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools/download
And Segger J-Link
    https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack

Flash the nRF52840 dongle
    nrfjprog -f nrf52 --chiperase --program ot-cli-ftd.hex --reset

## otbr Docker
Build the Docker image
    $ git clone https://github.com/openthread/ot-br-posix
    $ cd ot-br-posix
    $ git submodule update --init
    $ sudo docker build --no-cache -t openthread/otbr -f etc/docker/Dockerfile .