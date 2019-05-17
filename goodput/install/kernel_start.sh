#!/bin/bash

# Skip enumeration/discovery if any parameter is specified for this script.

if [ -z "$1" ]; then
	modprobe rio-scan scan=1
fi

modprobe rio_mport_cdev
modprobe rio-cm

exit 0
