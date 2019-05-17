#!/bin/bash

#  This script shuts down all RIO cluster nodes.

#  Warning:
#  ========
#  Please note that the master node (one where this script is executed)
#  must be last entry in the list of nodes.

RIO_CLASS_MPORT_DIR=/sys/class/rio_mport/rio_mport0

. /etc/rapidio/nodelist.sh

MY_NODE='';
MY_DESTID=$(cat $RIO_CLASS_MPORT_DIR/device/port_destid 2>/dev/null);

NODE_LIST='';
for node in $NODES; do
        DESTID=$(ssh "$USERID"@"$node" "cat $RIO_CLASS_MPORT_DIR/device/port_destid 2>/dev/null")
        [ $MY_DESTID = $DESTID ] && { MY_NODE="$node"; continue; }

        NODE_LIST="$NODE_LIST $node";
done
NODE_LIST="$NODE_LIST $MY_NODE";

echo "Reordered node list: $NODE_LIST";

for node in $NODE_LIST
do
	echo Shutdown $node
	ssh "$USERID"@"$node" poweroff
done

exit 0
