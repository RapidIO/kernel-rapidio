#!/bin/bash

#  This script creates all read/write goodput script files for
#  OBDIO Latency measurements.
#
#  This includes individual scripts for 1 byte up to 8 transfers,
#  for both reads and writes, as well as 2 scripts that will invoke
#  all of the individual scripts.
#
#  The "template" files in this directory is the basis of the
#  individual scripts.
#  

cd "$(dirname "$0")"
printf "\nCreating OBWIN LATENCY SCRIPTS\n\n"

shopt -s nullglob

DIR_NAME=obwin_lat

# SIZE_NAME is the file name
# SIZE is the hexadecimal representation of SIZE_NAME
#
# The two arrays must match up...

SIZE_NAME=(1B 2B 4B 8B)

SIZE=("0x1" "0x2" "0x4" "0x8")

BYTES=("0x400000" "0x400000" "0x400000" "0x400000")

PREFIX=ol

if [ -z "$IBA_ADDR" ]; then
        if [ -n "$1" ]; then
                IBA_ADDR=$1
        else
                IBA_ADDR=0x20d800000
                LOC_PRINT_HEP=1
        fi
fi

if [ -z "$DID" ]; then
        if [ -n "$2" ]; then
                DID=$2
        else
                DID=1
                LOC_PRINT_HEP=1
        fi
fi

if [ -z "$TRANS" ]; then
        if [ -n "$3" ]; then
                TRANS=$3
        else
                TRANS=0
                LOC_PRINT_HEP=1
        fi
fi

if [ -z "$WAIT_TIME" ]; then
        if [ -n "$4" ]; then
                WAIT_TIME=$4
        else
                WAIT_TIME=60
                LOC_PRINT_HEP=1
        fi
fi

if [ -z "$MPORT_DIR" ]; then
        if [ -n "$5" ]; then
                MPORT_DIR=$5
        else
                MPORT_DIR='mport0'
                LOC_PRINT_HEP=1
        fi
fi

if [ -n "$LOC_PRINT_HEP" ]; then
        echo $'\nScript accepts the following parameters:'
        echo $'IBA_ADDR : Hex address of target window on DID'
        echo $'DID      : Device ID that this device sends to'
        echo $'Trans    : DMA transaction type'
        echo $'Wait     : Time in seconds to wait before showing performance\n'
        echo $'DIR      : Directory to use as home directory for scripts\n'
fi

# ensure hex values are correctly prefixed
if [[ $IBA_ADDR != 0x* ]] && [[ $IBA_ADDR != 0X* ]]; then
        IBA_ADDR=0x$IBA_ADDR
fi

echo 'OBWIN_LATENCY IBA_ADDR = ' $IBA_ADDR
echo 'OBWIN_LATENCY DID      = ' $DID
echo 'OBWIN_LATENCY TRANS    = ' $TRANS
echo 'OBWIN_LATENCY WAIT_TIME= ' $WAIT_TIME
echo 'OBWIN_LATENCY MPORT_DIR= ' $MPORT_DIR

unset LOC_PRINT_HEP

# Function to format file names.
# Format is xxZss.txt, where
# xx is the prefix
# Z is W for writes or R for reads, parameter 1
# ss is a string selected from the SIZE_NAME array, parameter 2

declare t_filename

function set_t_filename {
	t_filename=$PREFIX$1$2".txt"
}

function set_t_filename_r {
	set_t_filename "R" $1
}

function set_t_filename_w {
	set_t_filename "W" $1
}

function set_t_filename_t {
	set_t_filename "T" $1
}

declare -i max_name_idx=0
declare -i max_size_idx=0
declare -i max_bytes_idx=0
declare -i idx=0

for name in "${SIZE_NAME[@]}"
do
	max_name_idx=($max_name_idx)+1;
done

for sz in "${SIZE[@]}"
do
	max_size_idx=($max_size_idx)+1;
done

for sz in "${BYTES[@]}"
do
	max_bytes_idx=($max_bytes_idx)+1;
done

if [ "$max_name_idx" != "$max_size_idx" ]; then
	echo "Mast name idx "$max_name_idx" not equal to max size idx "$max_size_idx
	exit 1
fi;

if [ "$max_name_idx" != "$max_bytes_idx" ]; then
	echo "Mast name idx "$max_name_idx" not equal to max bytes idx "$max_bytes_idx
	exit 1
fi;

echo $'\nArrays declared correctly...'

## Create files for DIOT commands, transmit latency

idx=0
while [ "$idx" -lt "$max_name_idx" ]
do
	declare filename
	declare w_filename

	set_t_filename_r ${SIZE_NAME[idx]}
	filename=$t_filename
	set_t_filename_w ${SIZE_NAME[idx]}
	w_filename=$t_filename
	cp tx_template $filename
	sed -i -- 's/acc_size/'${SIZE[idx]}'/g' $filename
	sed -i -- 's/bytes/'${BYTES[idx]}'/g' $filename
	cp $filename $w_filename
	idx=($idx)+1
done

## Create files for DIOR commands, Target of latency

idx=0
while [ "$idx" -lt "$max_name_idx" ]
do
	declare filename
	declare w_filename

	set_t_filename_t ${SIZE_NAME[idx]}
	filename=$t_filename
	cp rx_template $filename
	sed -i -- 's/size_name/'${SIZE_NAME[idx]}'/g' $filename
	sed -i -- 's/mport_dir/'${MPORT_DIR}'/g' $filename
	sed -i -- 's/acc_size/'${SIZE[idx]}'/g' $filename
	sed -i -- 's/bytes/'${BYTES[idx]}'/g' $filename
	idx=($idx)+1
done

sed -i -- 's/iba_addr/'$IBA_ADDR'/g' $PREFIX*.txt
sed -i -- 's/did/'$DID'/g' $PREFIX*.txt
sed -i -- 's/trans/'$TRANS'/g' $PREFIX*.txt
sed -i -- 's/wait_time/'$WAIT_TIME'/g' $PREFIX*.txt
sed -i -- 's/wr/1/g' ${PREFIX}W*.txt
sed -i -- 's/wr/0/g' ${PREFIX}R*.txt

## now create the "run all scripts" script files...


direction="read"
scriptname="../"$DIR_NAME"_"$direction 

echo "// This script runs all "$DIR_NAME $direction" scripts." > $scriptname
echo "log logs/"$MPORT_DIR/$DIR_NAME"_"$direction".log" >> $scriptname
echo "scrp ${MPORT_DIR}/${DIR_NAME}" >> $scriptname

idx=0
while [ "$idx" -lt "$max_name_idx" ]
do
	set_t_filename_r ${SIZE_NAME[idx]}

	echo "kill all" >> $scriptname
	echo "sleep 1"  >> $scriptname
	echo ". "$t_filename >> $scriptname
	idx=($idx)+1
done
echo "close" >> $scriptname
echo "scrp ${MPORT_DIR}" >> $scriptname

ls ../$DIR_NAME*
