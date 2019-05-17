#!/bin/bash
#
# This script checks the state of all nodes for 
# - kernel module status
# - RapidIO daemon processes

RIO_CLASS_MPORT_DIR=/sys/class/rio_mport/rio_mport0

. /etc/rapidio/nodelist.sh

OK=1	# Set OK to true before the checks

echo ' '

#For each node check that all is well
for node in $NODES
do
	# Check that the node was properly enumerated
	RIODEVS=$(ssh "$USERID"@"$node" "ls /sys/bus/rapidio/devices/")
	if [ -z "$RIODEVS" ]
	then
		RIODEVS="NOT ENUMERATED!"
		OK=0
	else
		RIODEVS="Devices: "$RIODEVS
	fi
	# Display node name, and the 'destid' for the node
	DESTID=$(ssh "$USERID"@"$node" "cat $RIO_CLASS_MPORT_DIR/device/port_destid 2>/dev/null")
	echo "+++ $node +++   DestID:" $DESTID $RIODEVS

	# Check that rio_mport_cdev was loaded
	RIO_MPORT_CDEV=$(ssh "$USERID"@"$node" "lsmod | grep rio_mport_cdev")
	if [ -z "$RIO_MPORT_CDEV" ]
	then
		echo "   rio_mport_cdev *NOT* loaded"
		OK=0
	else
		echo "   rio_mport_cdev       loaded"
	fi

	# Check that rio_cm was loaded
	RIO_CM=$(ssh "$USERID"@"$node" "lsmod | grep rio_cm")
	if [ -z "$RIO_CM" ]
	then
		echo "   rio_cm         *NOT* loaded"
		OK=0
	else
		echo "   rio_cm               loaded"
	fi

	# Check that fmd is running
	FMD_PID=$(ssh "$USERID"@"$node" pgrep -x fmd)
	if [ -z "$FMD_PID" ]
	then
		echo "   FMD            *NOT* running"
		OK=0
	else
		echo "   FMD                  running, PID=$FMD_PID"
	fi
done

exit 0

