#!/bin/bash

## This script creates two files:
## lat_pass.uout indicates success counts for all lines in the summary file
## lat_fail.uout indicates information about failures
##
## The script only passes if lat_fail.uout does not exist, and 
## lat_pass.uout does exist and is non-empty.
##
## This script checks that the throughput numbers computed are expected.
#3 There are four kinds of lines:
##
## CHECK input file list: Processing latency for file :
## - 4 lines, one line for each group of performance figures, 
##
## CHECK output file list: Output filename is             :
## - 4 lines, one line for each group of performance figures, 
##
## CHECK Header: SIZE .. Mbps...
## - 11 lines, one line for each group of performance figures, 
##
## CHECK DATA :size mbps gbps linkocc availcpu usercpu kernelcpu cpu OCC %
## Totals for each SIZE value add up
## There should never be a line with 
## - UserCPU, KernCPU, CPU OCC all 0
## - MBps/Gbps/LinkOcc all 0 
## 
## Also check individual log files for the following keywords,
##   which indicate failure:
## Unknown
## FAILED

PRINT_HELP=0

if [ -n "$1" ]
  then
    FILENAME=$1
else
        PRINT_HELP=1
fi

if [ $PRINT_HELP != "0" ]; then
        echo $'\nScript requires the following parameters:'
        echo $'FILENAME : Name of file in local directory containing'
        echo $'           output of summ_lat_logs.sh for all latency'
        echo $'           scripts.\n'
        echo $'Once complete, two files will be present in the directory'
        echo $'lat_pass.uout: File with all counts which passed.'
        echo $'lat_fail.uout: File with all counts and lines which failed'
        exit 1
fi;

SIZE_STRINGS=( XOutput XProcessing XSIZE
	"X0x1"
	"X0x2"
	"X0x4"
	"X0x8"
	"X0x10"
	"X0x20"
	"X0x40"
	"X0x80"
	"X0x100"
	"X0x200"
	"X0x400"
	"X0x800"
	"X0x1000"
	"X0x2000"
	"X0x4000"
	"X0x8000" 
	"X0x10000"
	"X0x20000"
	"X0x40000"
	"X0x80000"
	"X0x100000"
	"X0x200000"
	"X0x400000"
	"X0x18" )

# values copied from check_lat_logs.sh, need to be updated
SIZE_COUNT=( 5 5 5
	4 4 4 4
	2 3 3 3
	3 3 3 3
	3 2 2 2
	2 2 2 2
	2 2 2
	1 )


IDX=0

PASS=lat_pass.uout
FAIL=lat_fail.uout

rm -f $PASS
rm -f $FAIL

if [ ! -s ${FILENAME} ]; then
        echo "${FILENAME} is empty or does not exist!" > $FAIL
        exit
fi

sed -e 's/^ *//' ${FILENAME} > ${FILENAME}.temp
sed -i 's/^/X/' ${FILENAME}.temp 

# Check that expected number of lines is present
for SIZE in ${SIZE_STRINGS[@]}; do
	CNT="$( grep "${SIZE} " ${FILENAME}.temp | wc -l )"
	if [ ${CNT} == ${SIZE_COUNT[IDX]} ]; then
		echo "$SIZE $CNT ${SIZE_COUNT[IDX]}" >> ${PASS}
	else 
		echo "SIZE ${SIZE//X/} GOT $CNT EXP ${SIZE_COUNT[IDX]} FAIL" >> ${FAIL}
	fi;
	let "IDX = $IDX + 1"
done 

rm -f ${FILENAME}.temp

# Check that there are no lines with illegal values

CNT="$( grep " 0 " ${FILENAME} | wc -l )"
if [ "$CNT" -ne "0" ]; then
	echo $'\nFAIL: ZERO Transfers\n' >> ${FAIL}
	grep " 0 " ${FILENAME} >> ${FAIL}
fi;

CNT="$( grep " 0\.0" ${FILENAME} | wc -l )"
if [ "$CNT" -ne "0" ]; then
	echo $'\nFAIL: ZERO Min, Avg, or Max latency\n' >> ${FAIL}
	grep " 0\.0" ${FILENAME} >> ${FAIL}
fi

CNT="$( grep -E 'FAILED|Unknown|CRIT|ERR|WARN' *.log | wc -l )"
if [ "$CNT" -ne "0" ]; then
	echo $'\nFAIL: Keywords indicating errors exist in log files\n' >> ${FAIL}
fi

# ONLY PASS IF THE PASS FILE EXISTS AND THE FAIL FILE DOESN'T

if [ ! -s ${PASS} ]; then
	echo "FAILED, ${PASS} either does not exist or is empty!"
else
	if [ -f ${FAIL} ]; then
		echo "FAILED!"
		cat ${FAIL}
	else
		echo "PASSED!"
	fi
fi
