#!/bin/bash

## This script grabs the latency figures from a log file.
## It also displays the size of the transfer

MPNUM=0
unset NEWDIR

if [ -n "$1" ]; then
        MPNUM=$1
else
	echo 'usage: cleanup <mport> {<newdir>}'
	echo 'mport: Mport number where logs should be cleaned up.'
	echo 'newdir: Optional, move all logs to this directory newdir.'
	echo 'NOTE: If newdir is not specified, this script '
	echo '      DELETES all log, result, and out files'
	exit
fi

SUFFIXES=( log res out ulog ures uout )

if [ -n "$2" ]; then
        NEWDIR=$2
fi

if [[ ! -z "$NEWDIR" ]]; then
	mkdir -p $NEWDIR
	if [ "$?" -eq "0" ]; then
		for SUFFIX in "${SUFFIXES[@]}"; do
			mv mport${MPNUM}/*.${SUFFIX} ${NEWDIR}
		done
		exit
	else
		echo "could not create directory ${NEWDIR}"
		exit
	fi
fi

for SUFFIX in "${SUFFIXES[@]}"; do
	rm -f  mport${MPNUM}/*.${SUFFIX} ${NEWDIR}
done
