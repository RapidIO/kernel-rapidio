#!/bin/bash

#  This script creates all read/write goodput script files for
#  DMA throughput measurements.
#
#  This includes individual scripts for 1 byte up to 4MB transfers,
#  for both reads and writes, as well as 2 scripts that will invoke
#  all of the individual scripts.
#
#  The "template" file in this directory is the basis of the
#  individual scripts.
#  
cd "$(dirname "$0")"
printf "\nCreating SINGLE THREAD DMA THROUGHPUT SCRIPTS\n\n"

shopt -s nullglob

DIR_NAME=dma_thru_interleave

PREFIX=d1

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
		DID=0
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

if [ -z "$SYNC" ]; then
	if [ -n "$4" ]; then
		SYNC=$4
	else
		SYNC=0
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
	echo $'\nScript requires the following parameters:'
        echo $'IBA_ADDR : Hex address of target window on DID'
        echo $'DID      : Device ID of target device for performance scripts'
        echo $'Trans    : DMA transaction type'
        echo $'Wait     : Time in seconds to wait before taking performance measurement\n'
        echo $'Sync     : 0 - blocking, 1 - async, 2 - fire and forget\n'
        echo $'DIR      : Directory to use as home directory for scripts\n'
fi

# ensure hex values are correctly prefixed
if [[ $IBA_ADDR != 0x* ]] && [[ $IBA_ADDR != 0X* ]]; then
        IBA_ADDR=0x$IBA_ADDR
fi

echo $'\nDMA_THRUPUT IBA_ADDR = ' $IBA_ADDR
echo 'DMA_THRUPUT DID      = ' $DID
echo 'DMA_THRUPUT TRANS    = ' $TRANS
echo 'DMA_THRUPUT WAIT_TIME= ' $WAIT_TIME
echo 'DMA_THRUPUT MPORT_DIR= ' $MPORT_DIR

unset LOC_PRINT_HEP

# SIZE_NAME is the file name
# SIZE is the hexadecimal representation of SIZE_NAME
#
# The two arrays must match up...

SIZE_NAME=(
16B         32B         64B      128B       256B         512B     1K         2Ki
)

SIZE=(
"0x100000" "0x100000" "0x100000" "0x100000" "0x100000" "0x100000" "0x100000" "0x100000"
)

BYTES=(
"0x100000" "0x100000" "0x100000" "0x100000" "0x100000" "0x100000" "0x100000" "0x100000"
)

SSSIZE=(
"0x10"     "0x20"      "0x40"    "0x80"     "0x100"    "0x200"    "0x400"    "0x800"
)
SSDIST=(
"0x10"    "0x10"     "0x10"   "0x10"    "0x10"    "0x10"    "0x10"    "0x10"
)

DSDIST=(
"0"        "0"         "0"       "0"        "0"        "0"        "0"        "0"
)
DSSIZE=(
"0"        "0"         "0"       "0"        "0"        "0"        "0"        "0"
)

ADDR_OFFSET=(
"00"       "00"        "00"      "00"       "00"       "00"       "00"       "00"
)

# Function to format file names.
# Format is xxZssddzzDDZZOO.txt, where
# xx is the prefix
# Z is W for writes or R for reads, parameter 1
# ss is a string selected from the SIZE_NAME array, parameter 2
# dd is source stride distance
# zz is source stride size
# DD is dest stride distance
# ZZ is dest stride size
# OO is the address offset


declare t_filename

function set_t_filename {
	t_filename=$PREFIX$1$2$3$4$5$6$7".txt"
}

function set_t_filename_r {
	set_t_filename "R" $1 $2 $3 $4 $5 $6
}

function set_t_filename_w {
	set_t_filename "W" $1 $2 $3 $4 $5 $6
}

declare -i max_name_idx=0
declare -i max_size_idx=0
declare -i max_bytes_idx=0
declare -i max_sssize_idx=0
declare -i max_ssdist_idx=0
declare -i max_dssize_idx=0
declare -i max_dsdist_idx=0
declare -i max_addroffset_idx=0
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

for sz in "${SSSIZE[@]}"
do
	max_sssize_idx=($max_sssize_idx)+1;
done

for sz in "${SSDIST[@]}"
do
	max_ssdist_idx=($max_ssdist_idx)+1;
done

for sz in "${DSSIZE[@]}"
do
	max_dssize_idx=($max_dssize_idx)+1;
done

for sz in "${DSDIST[@]}"
do
	max_dsdist_idx=($max_dsdist_idx)+1;
done

for sz in "${ADDR_OFFSET[@]}"
do
	max_addroffset_idx=($max_addroffset_idx)+1;
done

if [ "$max_name_idx" != "$max_size_idx" ]; then
	echo "Mast name idx "$max_name_idx" not equal to max size idx "$max_size_idx
	exit 1
fi;

if [ "$max_name_idx" != "$max_bytes_idx" ]; then
	echo "Mast name idx "$max_name_idx" not equal to max bytes idx "$max_bytes_idx
	exit 1
fi;

if [ "$max_name_idx" != "$max_sssize_idx" ]; then
	echo "Mast name idx "$max_name_idx" not equal to source stride size idx "$max_sssize_idx
	exit 1
fi;

if [ "$max_name_idx" != "$max_ssdist_idx" ]; then
	echo "Mast name idx "$max_name_idx" not equal to source stride dist idx "$max_ssdist_idx
	exit 1
fi;

if [ "$max_name_idx" != "$max_dssize_idx" ]; then
	echo "Mast name idx "$max_name_idx" not equal to dest stride size idx "$max_dssize_idx
	exit 1
fi;

if [ "$max_name_idx" != "$max_dsdist_idx" ]; then
	echo "Mast name idx "$max_name_idx" not equal to dest stride dist idx "$max_dsdist_idx
	exit 1
fi;

if [ "$max_name_idx" != "$max_addroffset_idx" ]; then
	echo "Mast name idx "$max_name_idx" not equal to address offset idx "$max_addroffset_idx
	exit 1
fi;

echo "Arrays declared correctly..."

idx=0
while [ "$idx" -lt "$max_name_idx" ]
do
	declare filename
	declare w_filename

	set_t_filename_r ${SIZE_NAME[idx]} ${SSSIZE[idx]} ${SSDIST[idx]} ${DSSIZE[idx]} ${DSDIST[idx]} ${ADDR_OFFSET[idx]}
	filename=$t_filename
	set_t_filename_w ${SIZE_NAME[idx]} ${SSSIZE[idx]} ${SSDIST[idx]} ${DSSIZE[idx]} ${DSDIST[idx]} ${ADDR_OFFSET[idx]}
	w_filename=$t_filename
	cp template $filename
	((ADDR=IBA_ADDR+ADDR_OFFSET[idx]))
	printf -v HEX_ADDR "0x%x" $ADDR
	sed -i -- 's/acc_size/'${SIZE[idx]}'/g' $filename
	sed -i -- 's/bytes/'${BYTES[idx]}'/g' $filename
	sed -i -- 's/ssdist/'${SSDIST[idx]}'/g' $filename
	sed -i -- 's/sssize/'${SSSIZE[idx]}'/g' $filename
	sed -i -- 's/dsdist/'${DSDIST[idx]}'/g' $filename
	sed -i -- 's/dssize/'${DSSIZE[idx]}'/g' $filename
	sed -i -- 's/iba_addr/'${HEX_ADDR}'/g'         $filename
	cp $filename $w_filename
	idx=($idx)+1
done

sed -i -- 's/did/'$DID'/g' $PREFIX*.txt
sed -i -- 's/trans/'$TRANS'/g' $PREFIX*.txt
sed -i -- 's/wait_time/'$WAIT_TIME'/g' $PREFIX*.txt
sed -i -- 's/sync/'$SYNC'/g' $PREFIX*.txt
sed -i -- 's/wr/1/g' ${PREFIX}W*.txt
sed -i -- 's/wr/0/g' ${PREFIX}R*.txt

## now create the "run all scripts" script files...

DIR=('read' 'write')
declare -a file_list

for direction in "${DIR[@]}"
do
	scriptname="../"$DIR_NAME"_"$direction 

	echo "// This script runs all "$DIR_NAME $direction" scripts." > $scriptname
	echo "log logs/"$MPORT_DIR/$DIR_NAME"_"$direction".log" >> $scriptname
	echo "scrp ${MPORT_DIR}/$DIR_NAME" >> $scriptname

	idx=0
	while [ "$idx" -lt "$max_name_idx" ]
	do
		if [ "$direction" == "${DIR[0]}" ]; then
			set_t_filename_r ${SIZE_NAME[idx]} ${SSSIZE[idx]} ${SSDIST[idx]} ${DSSIZE[idx]} ${DSDIST[idx]} ${ADDR_OFFSET[idx]}
		else
			set_t_filename_w ${SIZE_NAME[idx]} ${SSSIZE[idx]} ${SSDIST[idx]} ${DSSIZE[idx]} ${DSDIST[idx]} ${ADDR_OFFSET[idx]}
		fi
		
		echo "kill all"          >> $scriptname
		echo "sleep "$WAIT_TIME  >> $scriptname
		echo ". "$t_filename >> $scriptname
		idx=($idx)+1
	done
	echo "close" >> $scriptname
	echo "scrp ${MPORT_DIR}" >> $scriptname
done

ls ../$DIR_NAME*
