#!/bin/bash

WAIT_TIME=30
DID=0
TRANS=0
IBA_ADDR=0x20d800000
ACC_SIZE=0x40000
BUFC=0x100 
STS=0x100 
BYTES=0x400000
SKT_PREFIX=234
MPORT=0
SYNC=0
SYNC2=$SYNC
SYNC3=$SYNC
PRINT_HELP=0

if [ -n "$1" ]
  then
    WAIT_TIME=$1
else
	PRINT_HELP=1
fi

if [ -n "$2" ]
  then
    TRANS=$2
else
	PRINT_HELP=1
fi

if [ -n "$3" ]
  then
    SYNC=$3
else
	PRINT_HELP=1
fi

if [ -n "$4" ]
  then
    DID=$4
else
	PRINT_HELP=1
fi

if [ -n "$5" ]
  then
    IBA_ADDR=$5
else
	PRINT_HELP=1
fi

if [ -n "$6" ]
  then
    SKT_PREFIX=$6
else
	PRINT_HELP=1
fi

if [ -n "$7" ]
  then
    MPORT=$7
fi

if [ -n "$8" ]
  then
    SYNC2=$8
else
	SYNC2=$SYNC
fi

if [ -n "$9" ]
  then
    SYNC3=$9
else
	SYNC3=$SYNC
fi

if [ $PRINT_HELP != "0" ]; then
	echo $'\nScript requires the following parameters:'
	echo $'WAIT       : Time in seconds to wait before perf measurement'
	echo $'DMA_TRANS  : DMA transaction type'
	echo $'             0 NW, 1 SW, 2 NW_R, 3 SW_R 4 NW_R_ALL'
	echo $'DMA_SYNC   : 0 - blocking, 1 - async, 2 - fire and forget'
	echo $'DID        : Device ID of target device for performance scripts'
	echo $'IBA_ADDR   : Hex RapidIO address of inbound window on DID'
	echo $'SKT_PREFIX : First 3 digits of 4 digit socket numbers'
	echo $'\nOptional parameters, if not entered same as DMA_SYNC'
	echo $'MPORT      : Mport number for these scripts, default is 0'
	echo $'DMA_SYNC2  : 0 - blocking, 1 - async, 2 - fire and forget'
	echo $'DMA_SYNC3  : 0 - blocking, 1 - async, 2 - fire and forget'
	exit 1
fi;

# ensure hex values are correctly prefixed
if [[ $IBA_ADDR != 0x* ]] && [[ $IBA_ADDR != 0X* ]]; then
        IBA_ADDR=0x$IBA_ADDR
fi

INTERP_TRANS=(NW SW NW_R SW_R NW_R_ALL);
INTERP_SYNC=(BLOCK ASYNC FAF);

MPORT_DIR='mport'${MPORT}

echo GENERATING ALL PERFORMANCE SCRIPTS WITH
echo 'WAIT TIME  :' $WAIT_TIME SECONDS
echo 'TRANS      :' $TRANS ${INTERP_TRANS[TRANS]}
echo 'SYNC       :' $SYNC  ${INTERP_SYNC[SYNC]}
echo 'SYNC2      :' $SYNC2 ${INTERP_SYNC[SYNC2]}
echo 'SYNC3      :' $SYNC3 ${INTERP_SYNC[SYNC3]}
echo 'DID        :' $DID
echo 'MPORT      :' $MPORT
echo 'IBA_ADDR   :' $IBA_ADDR
echo 'SKT_PREFIX :' $SKT_PREFIX 
echo 'MPORT_DIR  :' $MPORT_DIR

cd ..

# Create mport specific directory for performance scripts,
# and generate the scripts.

mkdir -m 777 -p $MPORT_DIR
cp -r scripts/performance/* ${MPORT_DIR}

cd ${MPORT_DIR}/dma_thru
source create_scripts.sh $WAIT_TIME $DID $TRANS $IBA_ADDR $SYNC $MPORT_DIR
cd ../dma_thru_interleave
source create_scripts.sh $WAIT_TIME $DID $TRANS $IBA_ADDR $SYNC $MPORT_DIR
cd ../pdma_thru
source create_scripts.sh $WAIT_TIME $DID $TRANS $IBA_ADDR $SYNC $SYNC2 $SYNC3 $MPORT_DIR
cd ../dma_lat
source create_scripts.sh $IBA_ADDR $DID $TRANS $WAIT_TIME $MPORT_DIR
cd ../msg_thru
source create_scripts.sh $SKT_PREFIX $DID $WAIT_TIME $MPORT_DIR
cd ../obwin_thru
source create_scripts.sh $IBA_ADDR $DID $TRANS $WAIT_TIME $MPORT_DIR

cd ../obwin_lat
source create_scripts.sh $IBA_ADDR $DID $TRANS $WAIT_TIME $MPORT_DIR
cd ../msg_lat
source create_scripts.sh $SKT_PREFIX $DID $WAIT_TIME $MPORT_DIR
cd ../..
cp scripts/run_all_dma  ${MPORT_DIR}
sed -i -- 's/MPORT_DIR/'$MPORT_DIR'/g' ${MPORT_DIR}/run_all_dma
cp scripts/run_all_perf ${MPORT_DIR}
sed -i -- 's/MPORT_DIR/'$MPORT_DIR'/g' ${MPORT_DIR}/run_all_perf

find ${MPORT_DIR} -type f -perm 755 -exec chmod 777 {} \;
find ${MPORT_DIR} -type f -perm 775 -exec chmod 777 {} \;
find ${MPORT_DIR} -type d -perm 775 -exec chmod 777 {} \;
