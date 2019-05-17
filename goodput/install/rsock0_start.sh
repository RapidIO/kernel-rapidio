#!/bin/bash

SOURCE_PATH=/opt/rapidio/rapidio_sw
RIO_CLASS_MPORT_DIR=/sys/class/rio_mport/rio_mport0

. /etc/rapidio/nodelist.sh
. /etc/rapidio/riosocket_conf.sh

MTU='/bin/true';
[ -z "$1" ] || {
	MTU=$1;
	if (($MTU < 576 || $MTU > 65500)); then
		echo "Invalid MTU=$MTU. Must be between 576 and 65500." 1>&2
		exit 3;
	fi
	MTU="/sbin/ifconfig rsock0 mtu $MTU";
}

for node in $NODES; do
	DESTID=$(ssh "$USERID"@"$node" "cat $RIO_CLASS_MPORT_DIR/device/port_destid")
	echo $DESTID | grep -qi ffff && {
		echo "Node $node not enumerated? Destid is $DESTID" 2>&1
		exit 1;
	}

	if (( $DESTID == 0 )); then
		echo "Node $node has destid $DESTID which cannot yield a correct IPv4 addr. Use FMD. Skipping." 2>&1
		continue;
	fi

	ssh "$USERID"@"$node" "/sbin/lsmod" | grep -q riosocket && continue;

	ssh "$USERID"@"$node" "/sbin/modprobe riosocket; /sbin/lsmod" | grep -q riosocket || {
		echo "Node $node won't load riosocket kernel module" 2>&1
		exit 2;
	} 

	# XXX This IPv4 assignment is naive at best and works with up to 254 node clusters
	# XXX DESTID=0 will yield 169.254.0.0 which is bcast addr. Use FMD for enumeration.
	ssh "$USERID"@"$node" "$MTU; /sbin/ifconfig rsock0 $RIOSOCKET_IP$DESTID netmask $RIOSOCKET_MASK up"
done
