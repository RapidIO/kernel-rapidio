#!/bin/bash

#  This script creates all read/write goodput script files for
#  parallel DMA throughput measurements.
#
#  This includes individual scripts for 1 byte up to 4MB transfers,
#  for both reads and writes, as well as 2 scripts that will invoke
#  all of the individual scripts.
#
#  The script does this for an array of template file names and prefixes
#

cd "$(dirname "$0")"
printf "\nCreating PARALLEL DMA THROUGHPUT SCRIPTS\n\n"

shopt -s nullglob

DIR_NAME=pdma_thru

# SIZE_NAME is the file name
# SIZE is the hexadecimal representation of SIZE_NAME
# BYTES is the total amount of memory to write for the SIZE
#
# SIZE_NAME, SIZE, and BYTES entries must match up...

SIZE_NAME=(1B 2B 4B 8B 16B 32B 64B 128B 256B 512B
	1K 2K 4K 8K 16K 32K 64K 128K 256K 512K 
	1M 2M 4M)

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

## TEMPLATES is the name of the template files
## PREFIXES is the name of the parallel DMA tests created for each template
##
## Entries in TEMPLATES and PREFIXES must match

TEMPLATES=("template_diffdma"  "template_mix" "template_onedma")
PREFIXES=("pdd"                "pdm"           "pd1")

if [ -z "$WAIT_TIME" ]; then
        if [ -n "$1" ]; then
                WAIT_TIME=$1
        else
                WAIT_TIME=60
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

if [ -z "$IBA_ADDR" ]; then
        if [ -n "$4" ]; then
                IBA_ADDR=$4
        else
                IBA_ADDR=0x20d800000
                LOC_PRINT_HEP=1
        fi
fi

if [ -z "$SYNC" ]; then
        if [ -n "$5" ]; then
                SYNC=$5
        else
                SYNC=0
                LOC_PRINT_HEP=1
        fi
fi

if [ -z "$SYNC2" ]; then
        if [ -n "$6" ]; then
                SYNC2=$6
        else
                SYNC2=$SYNC
                LOC_PRINT_HEP=1
        fi
fi

if [ -z "$SYNC3" ]; then
        if [ -n "$7" ]; then
                SYNC3=$7
        else
                SYNC3=$SYNC
                LOC_PRINT_HEP=1
        fi
fi

if [ -z "$MPORT_DIR" ]; then
        if [ -n "$8" ]; then
                MPORT_DIR=$8
        else
                MPORT_DIR='mport0'
                LOC_PRINT_HEP=1
        fi
fi

if [ -n "$LOC_PRINT_HEP" ]; then
        echo $'\nScript requires the following parameters:'
        echo $'WAIT     : Time in seconds to wait before taking performance measurement'
        echo $'DID      : Device ID of target device for performance scripts'
        echo $'TRANS    : DMA transaction type'
        echo $'           0 NW, 1 SW, 2 NW_R, 3 SW_R 4 NW_R_ALL'
        echo $'IBA_ADDR : Hex address of target window on DID'
        echo $'DMA_SYNC : 0 - blocking, 1 - async, 2 - fire and forget'
        echo $'DMA_SYNC2: 0 - blocking, 1 - async, 2 - fire and forget'
        echo $'DMA_SYNC3: 0 - blocking, 1 - async, 2 - fire and forget\n'
        echo $'DIR      : Directory to use as home directory for scripts\n'
fi

# ensure hex values are correctly prefixed
if [[ $IBA_ADDR != 0x* ]] && [[ $IBA_ADDR != 0X* ]]; then
        IBA_ADDR=0x$IBA_ADDR
fi

echo PDMA_THRUPUT WAIT_TIME= $WAIT_TIME
echo PDMA_THRUPUT DID      = $DID
echo PDMA_THRUPUT TRANS    = $TRANS
echo PDMA_THRUPUT IBA_ADDR = $IBA_ADDR
echo PDMA_THRUPUT SYNC     = $SYNC
echo PDMA_THRUPUT SYNC2    = $SYNC2
echo PDMA_THRUPUT SYNC3    = $SYNC3
echo PDMA_THRUPUT DIR      = $MPORT_DIR

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

echo "Arrays declared correctly..."

TEMPLATE_IDX=0;
for template in "${TEMPLATES[@]}"
do
	PREFIX=${PREFIXES[TEMPLATE_IDX]}
	TEMPLATE_IDX=$TEMPLATE_IDX+1;
	idx=0
	while [ "$idx" -lt "$max_name_idx" ]
	do
		declare filename
		declare w_filename

		set_t_filename_r ${SIZE_NAME[idx]}
		filename=$t_filename
		set_t_filename_w ${SIZE_NAME[idx]}
		w_filename=$t_filename
		cp $template $filename
		sed -i -- 's/acc_size/'${SIZE[idx]}'/g' $filename
		sed -i -- 's/bytes/'${BYTES[idx]}'/g' $filename
		cp $filename $w_filename
		idx=($idx)+1
	done

	sed -i -- 's/iba_addr/'$IBA_ADDR'/g' $PREFIX*.txt
	sed -i -- 's/did/'$DID'/g' $PREFIX*.txt
	sed -i -- 's/trans/'$TRANS'/g' $PREFIX*.txt
	sed -i -- 's/wait_time/'$WAIT_TIME'/g' $PREFIX*.txt
	sed -i -- 's/sync2/'$SYNC2'/g' $PREFIX*.txt
	sed -i -- 's/sync3/'$SYNC3'/g' $PREFIX*.txt
	sed -i -- 's/sync/'$SYNC'/g' $PREFIX*.txt
	sed -i -- 's/wr/1/g' ${PREFIX}W*.txt
	sed -i -- 's/wr/0/g' ${PREFIX}R*.txt

	## now create the "run all scripts" script files...

	DIR=('read' 'write')
	declare -a file_list

	for direction in "${DIR[@]}"
	do
		file_set=$DIR_NAME"_"$PREFIX"_"$direction 
		scr_name="../"$file_set

		echo "// This script runs all "$file_set" scripts." > $scr_name
		echo "log logs/"$MPORT_DIR/$file_set".log" >> $scr_name
		echo "scrp ${MPORT_DIR}/${DIR_NAME}" >> $scr_name
	
		idx=0
		while [ "$idx" -lt "$max_name_idx" ]
		do
			if [ "$direction" == "${DIR[0]}" ]; then
				set_t_filename_r ${SIZE_NAME[idx]}
			else
				set_t_filename_w ${SIZE_NAME[idx]}
			fi

			echo "kill all"          >> $scr_name
			echo "sleep "$WAIT_TIME  >> $scr_name
			echo ". "$t_filename >> $scr_name
			idx=($idx)+1
		done
		echo "close" >> $scr_name
		echo "scrp ${MPORT_DIR}" >> $scr_name
	done
done

ls ../$DIR_NAME*
