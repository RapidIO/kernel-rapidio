#!/bin/bash

# Temporary script to speed up RSKT test apps development.

SOURCE_PATH=/opt/rapidio/rapidio_sw
SCRIPTS_PATH=$SOURCE_PATH/install
RIO_CLASS_MPORT_DIR=/sys/class/rio_mport/rio_mport0

. /etc/rapidio/nodelist.sh

REVNODES='';
for node in $NODES; do
  REVNODES="$node $REVNODES"
done

for node in $REVNODES
do
	# Kill FMD
	THE_PID=$(ssh "$USERID"@"$node" pgrep -x fmd)
	echo "Killing -fmd- on $node  FMD  PID=$THE_PID"
	for proc in $THE_PID
	do
		ssh "$USERID"@"$node" "kill -s 2 $proc"
	done

	sleep 2

	ssh "$USERID"@"$node" "rm -f /dev/shm/RIO_SM_DEV_DIR"
	ssh "$USERID"@"$node" "rm -f /dev/shm/RIO_SM_DEV_DIR_MUTEX"
done

