#!/bin/bash
#
# Remove all devices in /sys/bus/rapidio/devices

cd /sys/bus/rapidio/devices 

for f in *; do
        /opt/rapidio/rapidio_sw/common/libmport/riodp_test_devs -d -N "$f";
done

exit 0
