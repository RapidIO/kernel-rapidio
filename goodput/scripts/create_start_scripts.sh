#!/bin/bash

PRINT_HELP=0
SKT_PREFIX=234
IBA_ADDR=0x200000000

if [ -n "$1" ]
  then
    MPORT=$1
else
        PRINT_HELP=1
fi

if [ -n "$2" ]
  then
    SKT_PREFIX=$2
else
        PRINT_HELP=1
fi

if [ -n "$3" ]
  then
    IBA_ADDR=$3
else
        PRINT_HELP=1
fi

if [ $PRINT_HELP != "0" ]; then
        echo $'\nScript to create Goodput setup scripts.'
        echo $'\nScript requires the following parameters:'
        echo $'MPORT      : Mport number usually 0'
        echo $'SOCKET_PFX : First 3 digits of 4 digit socket numbers i.e. 123'
        echo $'IBA_ADDR   : Hex RapidIO address of inbound window on DID'
        echo $'\nNo parameters entered, scripts not generated...'
        exit 1
fi;

# ensure hex values are correctly prefixed
if [[ $IBA_ADDR != 0x* ]] && [[ $IBA_ADDR != 0X* ]]; then
        IBA_ADDR=0x$IBA_ADDR
fi

MPORT_DIR=mport${MPORT}

echo GENERATING GOODPUT START SCRIPTS WITH
echo 'MPORT      : ' $MPORT       
echo 'MPORT_DIR  : ' $MPORT_DIR
echo 'SOCKET_PFX : ' $SKT_PREFIX
echo 'IBA_ADDR   : ' $IBA_ADDR

cd ..

# Create mport specific directory for log files, include analysis scripts
mkdir -m 777 -p logs
chmod 777 logs
mkdir -m 777 -p logs/${MPORT_DIR}
chmod 777 logs/${MPORT_DIR}

cp logs/*.sh logs/${MPORT_DIR}/

find logs/${MPORT_DIR} -type f -perm 664 -exec chmod 666 {} \;
find logs/${MPORT_DIR} -type f -perm 775 -exec chmod 777 {} \;
find logs/${MPORT_DIR} -type d -perm 775 -exec chmod 777 {} \;

mkdir -m 777 -p $MPORT_DIR

cp -f 'scripts/template_st_targ' $MPORT_DIR/start_target
cp -f 'scripts/template_st_src' $MPORT_DIR/start_source

find ${MPORT_DIR} -type f -perm 664 -exec chmod 666 {} \;
find ${MPORT_DIR} -type f -perm 644 -exec chmod 666 {} \;
find ${MPORT_DIR} -type f -perm 775 -exec chmod 777 {} \;
find ${MPORT_DIR} -type d -perm 775 -exec chmod 777 {} \;

cd $MPORT_DIR

sed -i -- 's/MPORT_DIR/'$MPORT_DIR'/g' 'start_target'
sed -i -- 's/skt_prefix/'$SKT_PREFIX'/g' 'start_target'
sed -i -- 's/iba_addr/'$IBA_ADDR'/g' 'start_target'

sed -i -- 's/MPORT_DIR/'$MPORT_DIR'/g' 'start_source'
sed -i -- 's/iba_addr/'$IBA_ADDR'/g' 'start_source'

