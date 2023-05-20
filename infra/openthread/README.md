# openThread Border Router
See: https://openthread.io/guides/border-router

## Proxmox

### Load iptables
On the Proxmox host
    # modprobe ip6table_filter

### LXC USB passthrough
From: https://medium.com/@konpat/usb-passthrough-to-an-lxc-proxmox-15482674f11d

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

On the Proxmox host, edit the container config in /etc/pve/lxc/<container_id>.conf
    lxc.cgroup.devices.allow: c 189:* rwm
    lxc.mount.entry: /dev/bus/usb/003/002 dev/bus/usb/003/002 none bind,optional,create=file
Alternatively, mount the entire bus path (in case device changes):
    lxc.cgroup.devices.allow: c 189:* rwm
    lxc.mount.entry: /dev/bus/usb/003 dev/bus/usb/003 none bind,optional,create=dir

Restart the container, and look for RCP tty in the container to see if the device is visible.
    ls -l /dev/ttyACM*
