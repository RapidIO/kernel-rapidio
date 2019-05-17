#!/bin/bash

SOURCE_PATH=/opt/rapidio/rapidio_sw
RIO_CLASS_MPORT_DIR=/sys/class/rio_mport/rio_mport0

. /etc/rapidio/nodelist.sh

for node in $NODES; do
	DESTID=$(ssh "$USERID"@"$node" "cat $RIO_CLASS_MPORT_DIR/device/port_destid")
	echo $DESTID | grep -qi ffff && {
		echo "Node $node not enumerated? Destid is $DESTID" 2>&1
		continue;
	}

	ssh "$USERID"@"$node" "/sbin/ifconfig rsock0 down; /sbin/rmmod riosocket"
done
