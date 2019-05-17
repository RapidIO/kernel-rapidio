#!/bin/bash

# Unload kernel modules.

modprobe -r rio_mport_cdev
modprobe -r rio-cm

exit 0
