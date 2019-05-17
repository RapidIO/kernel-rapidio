#!/bin/bash

## This script grabs the latency figures from a log file.
## It also displays the size of the transfer

SUFFIX=log
OSUFFIX=res

if [ -n "$1" ]; then
        SUFFIX=$1
fi

if [ -n "$2" ]; then
        OSUFFIX=$2
fi

function summarize_latency_log {
	IN_FILE=$1

	TEMP_FILE1=$1'.temp1'
	TEMP_FILE2=$1'.temp2'
	TEMP_FILE3=$1'.temp3'
	OUT_FILE=$1'.'${OSUFFIX}

	echo $'\nProcessing latency log file : ' $IN_FILE 
	echo $'Output filename is          : ' $OUT_FILE 
	
	## Get performance measurements and labels
	awk '/0 Run     /{print}
	/echo/{print}' $IN_FILE  > $TEMP_FILE1
	
	### Concatenate groups of 2 lines together
	
	awk '
	BEGIN {getline; l1 = $0;}
	{print l1 " " $0; l1 = $0}
	' $TEMP_FILE1 > $TEMP_FILE2
	
	## Only keep lines with echo and Run 
	grep -E '(^echo.*Run)' $TEMP_FILE2 > $TEMP_FILE3
	
	## Reformat each line, and add a header...
	awk '
	BEGIN {print "SIZE     Transfers   Min_Usecs   Avg_Usecs   Max_Usecs"}
	{printf "%8s %9s   %9s   %9s   %9s\n", $4, $9, $10, $11, $12}
	' $TEMP_FILE3 > $OUT_FILE

	cat $OUT_FILE

	rm -f $TEMP_FILE1 $TEMP_FILE2 $TEMP_FILE3 
}

dir_list="$(ls *lat_*.${SUFFIX} | grep -v done)"

if [ ${#dir_list[@]} -eq 0 ]; then
        exit
fi

for f in ${dir_list[@]}; do
        summarize_latency_log ${f}
done
