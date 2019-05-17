#!/bin/bash

SOURCE_PATH=/opt/rapidio/rapidio_sw
RIO_CLASS_MPORT_DIR=/sys/class/rio_mport/rio_mport0

START_FMD=y

echo "$@" | grep -q nofmd  && START_FMD=n;

# set | grep ^START_

. /etc/rapidio/nodelist.sh

echo $'\nStarting the following daemons on all system nodes.'
echo "Enable/disable daemons by entering named keywords."

[ "$START_FMD" = 'y' ]    && echo "FMD and no kernel enum         (nofmd   to disable)"
[ "$START_FMD" = 'n' ]    && echo "RapidIO kernel enum required -- has $SOURCE_PATH/rio_start.sh been run?"
# Load drivers on each node -- unless nofmd specified
if [ "$START_FMD" = 'y' ]; then
	$SOURCE_PATH/rio_start.sh noenum

        for node in $NODES; do
                DESTID=$(ssh "$USERID"@"$node" "cat $RIO_CLASS_MPORT_DIR/device/port_destid")
                echo "Starting fmd on $node destID=$DESTID"
                ssh "$USERID"@"$node" screen -dmS fmd $SOURCE_PATH/fabric_management/daemon/fmd -l 3
                sleep 1
                FMD_PID=$(ssh "$USERID"@"$node" pgrep -x fmd)
                echo "$node fmd pid=$FMD_PID"
        sleep 1; # Allow FMD to enumerate nodes
        done
fi
