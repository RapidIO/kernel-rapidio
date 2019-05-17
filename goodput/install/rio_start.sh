#!/bin/bash

#  This script starts all RIO cluster nodes.

#  Warning:
#  ======== 
#  Please note that the master node (one where this script is executed)
#+ must be first entry in the list of nodes.

SOURCE_PATH="/opt/rapidio/rapidio_sw"
INSTALL_PATH=$SOURCE_PATH"/install"

. /etc/rapidio/nodelist.sh

let c=0;
for node in $NODES
do
	let c=c+1;
	echo "${node}"

	if [ -z "$1" ]
	then
		ssh "$USERID"@"${node}" $INSTALL_PATH/kernel_start.sh
	else
		ssh "$USERID"@"${node}" $INSTALL_PATH/kernel_start.sh noenum
	fi

	if [ $c -eq 1 ]
	then
		sleep 5
	fi
done
