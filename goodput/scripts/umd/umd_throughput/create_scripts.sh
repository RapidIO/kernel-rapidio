#!/bin/bash

#  This script creates all goodput script files for
#  umd DMA throughput measurements.
#
#  This includes individual scripts for 1 byte up to 4MB transfers, 1 to 8 threads and 1 to 4 DMA engines
#
#  The "template" file in this directory is the basis of the
#  individual scripts.
#
cd "$(dirname "$0")"
printf "\nCreating UMD DMA THROUGHPUT SCRIPTS\n\n"

shopt -s nullglob

DIR_NAME=umd_throughput

PREFIX=d1_

WAIT_TIME=10

if [ -z "$IBA_ADDR" ]; then
	if [ -n "$1" ]; then
		IBA_ADDR=$1
	else
		IBA_ADDR=0x26b000000
		LOC_PRINT_HEP=1
	fi
fi

if [ -z "$DID" ]; then
	if [ -n "$2" ]; then
		DID=$2
	else
		DID=0
		LOC_PRINT_HEP=1
	fi
fi


if [ -z "$LOG_PATH" ]; then
	if [ -n "$3" ]; then
		LOG_PATH=$3
	else
		LOG_PATH="scripts/umd/umd_throughput/"
		LOC_PRINT_HEP=1
	fi
fi

if [ -n "$LOC_PRINT_HEP" ]; then
	echo $'\nScript requires the following parameters:'
	echo $'IBA_ADDR : Hex address of target window on DID'
	echo $'DID      : Device ID of target device for performance scripts'
	echo $'DIR      : Directory to use as home directory for scripts\n'
fi

# ensure hex values are correctly prefixed
if [[ $IBA_ADDR != 0x* ]] && [[ $IBA_ADDR != 0X* ]]; then
	IBA_ADDR=0x$IBA_ADDR
fi

echo $'\nDMA_THRUPUT IBA_ADDR = ' $IBA_ADDR
echo 'DMA_THRUPUT DID      = ' $DID

unset LOC_PRINT_HEP

#File name is the combination of SIZE_NAME,  THREAD_NUM_NAME and DMA_NUM_NAME
# # SIZE is the hexadecimal representation of SIZE_NAME
#
# The two arrays must match up...

SIZE_NAME=(1B 2B 4B 8B 16B 32B 64B 128B 256B 512B
	1K 2K 4K 8K 16K 32K 64K 128K 256K 512K 
	1M 2M 4M)

THREAD_NUM_NAME=(1T 2T 3T 4T 5T 6T 7T 8T)

DMA_NUM_NAME=(1D 2D 3D 4D)

SIZE=(
"0x1" "0x2" "0x4" "0x8"
"0x10" "0x20" "0x40" "0x80"
"0x100" "0x200" "0x400" "0x800"
"0x1000" "0x2000" "0x4000" "0x8000"
"0x10000" "0x20000" "0x40000" "0x80000"
"0x100000" "0x200000" "0x400000")

BYTES=(
"0x10000" "0x10000" "0x10000" "0x10000"
"0x10000" "0x10000" "0x10000" "0x10000"
"0x10000" "0x10000" "0x10000" "0x10000"
"0x10000" "0x10000" "0x10000" "0x10000"
"0x10000" "0x20000" "0x40000" "0x80000"
"0x100000" "0x200000" "0x400000")


THREAD_ID=(0 1 2 3 4 5 6 7)

DMA_MASK=(0x01 0x03 0x07 0xF)

# Function to format file names.
# Format is xxZss.txt, where
# xx is the prefix
# ss is a string selected from the SIZE_NAME array, parameter 2

declare t_filename

function set_t_filename {
	t_filename=$PREFIX$1
}

function set_filename {
	set_t_filename $1
}

function gen_file {
	local filename_prefix=$1
    local filename=${filename_prefix}.txt
	local size=${SIZE[$2]}
	local bytes=${BYTES[$2]}
	local dma_engines=$4

	logfile="$LOG_PATH${filename_prefix}.log"

	let "threads = $3 + 1"
	let "thread_max = $threads - 1"

	echo "Generating file $filename with size $size threads $threads dma_engines $dma_engines"
	echo "log $logfile" > $filename
	echo "Uopen" >> $filename
	echo "Uconfig ${DMA_MASK[$dma_engines]}" >> $filename
	echo "Ustart" >> $filename
	echo >> $filename

	echo >> $filename

	for i in `seq 0 $thread_max`;
	do
	    echo "thread ${i} -1 0" >> $filename
	done

	echo >> $filename

	for i in `seq 0 $thread_max`;
	do
	    echo "wait ${i} H" >> $filename
	done

	echo >> $filename

	for i in `seq 0 $thread_max`;
	do
	    echo "Udma ${i} ${DID} ${IBA_ADDR} ${bytes} ${size}" >> $filename
	done

	echo >> $filename

	echo "sleep 60" >> $filename
	echo "goodput" >> $filename

	echo "halt all" >> $filename
	for i in `seq 0 $thread_max`;
	do
	    echo "wait ${i} H" >> $filename
	done

	echo "kill all" >> $filename
	for i in `seq 0 $thread_max`;
	do
	    echo "wait ${i} D" >> $filename
	done

	echo "Ustop" >> $filename
	echo "Uclose" >> $filename
	echo "close" >> $filename
}

declare -i max_name_idx=0
declare -i max_size_idx=0
declare -i max_bytes_idx=0
declare -i max_thread_id=0
declare	-i max_dma_idx=0
declare -i idx=0
declare -i id=0
declare -i dma_id=0

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

for id in "${THREAD_ID[@]}"
do
	max_thread_id=($max_thread_id)+1;
done
echo "MAX thread "$max_thread_id""

for dma_id in "${DMA_MASK[@]}"
do
	max_dma_idx=($max_dma_idx)+1;
done
echo "MAX DMA "$max_dma_idx""

if [ "$max_name_idx" != "$max_size_idx" ]; then
	echo "Mast name idx "$max_name_idx" not equal to max size idx "$max_size_idx
	exit 1
fi;

if [ "$max_name_idx" != "$max_bytes_idx" ]; then
	echo "Mast name idx "$max_name_idx" not equal to max bytes idx "$max_bytes_idx
	exit 1
fi;

echo "Arrays declared correctly..."

scriptname="../"$DIR_NAME".txt"
echo "// This script runs all "$DIR_NAME" scripts." > $scriptname
echo "scrp scripts/umd/umd_throughput/" >> $scriptname

idx=0
while [ "$idx" -lt "$max_size_idx" ]
do
	declare filename

	set_filename ${SIZE_NAME[idx]}
	filename=$t_filename
	id=0
	while [ "$id" -lt "$max_thread_id" ]
	do
		declare filename_t
		declare -i temp_max_dma_idx
		declare -i temp_id
		filename_t=$filename${THREAD_NUM_NAME[id]}
		temp_id=($id)+1
		if [ $max_dma_idx -ge $temp_id ]; then
			temp_max_dma_idx=$temp_id
		else
			temp_max_dma_idx=$max_dma_idx
		fi

		dma_id=0
		while [ "$dma_id" -lt "$temp_max_dma_idx" ]
		do
			declare filename_dt
			filename_dt="${filename_t}${DMA_NUM_NAME[$dma_id]}"
			gen_file $filename_dt $idx $id $dma_id

			echo "kill all"          >> $scriptname
			echo "sleep "$WAIT_TIME  >> $scriptname
			echo ". "$filename_dt".txt" >> $scriptname

			dma_id=($dma_id)+1
		done
		id=($id)+1
	done
	idx=($idx)+1
done

sed -i -- 's/iba_addr/'$IBA_ADDR'/g' $PREFIX*.txt
sed -i -- 's/did/'$DID'/g' $PREFIX*.txt


ls ../$DIR_NAME*
