#!/bin/bash

# Log regression script to execute all goodput measurements on two nodes.
#
# First node entered is the master node, where requests originate.
# Second node entered is the slave node, where requests are terminated.
# Optionally, a directory where the goodput source code resides on both
#   nodes can be entered.  The directory default value is the parent
#   directory of the directory containing this script.
# 
# There are many steps to this script.

MAST_NODE=node1
SLAVE_NODE=node2

MAST_MPNUM=0
SLAVE_MPNUM=0

DFLT_HOMEDIR=$PWD
PREVDIR=/..

DFLT_HOMEDIR=$DFLT_HOMEDIR$PREVDIR
DFLT_WAIT_TIME=30
DFLT_IBA_ADDR=0x200000000
DFLT_SKT_PREFIX=234

HOMEDIR=$DFLT_HOMEDIR

WAIT_TIME=$DFLT_WAIT_TIME
TRANS_IN=5
IBA_ADDR=$DFLT_IBA_ADDR
ACC_SIZE=0x40000
BYTES=0x400000
SKT_PREFIX=$DFLT_SKT_PREFIX
SYNC_IN=3
SYNC2_IN=${SYNC_IN}
SYNC3_IN=${SYNC_IN}

PRINT_HELP=0

if [ -n "$1" ]
  then
    MAST_NODE=$1
else
	PRINT_HELP=1
fi

if [ -n "$2" ]
  then
    SLAVE_NODE=$2
else
	PRINT_HELP=1
fi

if [ -n "$3" ]
  then
    MAST_MPNUM=$3
	shift
else
	PRINT_HELP=1
fi

if [ -n "$3" ]
  then
    SLAVE_MPNUM=$3
	shift
else
	PRINT_HELP=1
fi

if [ -n "$3" ]
  then
    HOMEDIR=$3
    shift
fi

if [ -n "$3" ]
  then
    WAIT_TIME=$3
    shift
fi

if [ -n "$3" ]
  then
    TRANS_IN=$3
    shift
fi

if [ -n "$3" ]
  then
    SYNC_IN=$3
    shift
fi

if [ -n "$3" ]
  then
    IBA_ADDR=$3
    shift
fi

if [ -n "$3" ]
  then
    SKT_PREFIX=$3
    shift
fi

if [ -n "$3" ]
  then
    SYNC2_IN=$3
    shift
fi

if [ -n "$3" ]
  then
    SYNC3_IN=$3
    shift
fi

if [ $PRINT_HELP != "0" ]; then
	echo $'\nScript requires the following parameters:'
	echo $'MAST       : Name of master node/IP address'
	echo $'SLAVE      : Name of slave node/IP address'
	echo $'MAST_MPNUM : Master node mport number (usually 0)'
	echo $'SLAVE_MPNUM: Slave node mport number (usually 0)'
	echo $'All parameters after this are optional.  Default values shown.'
	echo $'DIR        : Directory on both MAST and SLAVE to run tests.'
	echo $'             Default is ' $DFLT_HOMEDIR
    echo $'WAIT       : Time in seconds to wait before performance measurement'
	echo $'             Default is ' $DFLT_WAIT_TIME
    echo $'DMA_TRANS  : DMA transaction type'
    echo $'             0 NW, 1 SW, 2 NW_R, 3 SW_R, 4 NW_R_ALL'
	echo $'             Default is 0 NW.'
    echo $'DMA_SYNC   : 0 - blocking, 1 - async, 2 - fire and forget'
	echo $'             Default is 0 blocking.'
    echo $'IBA_ADDR   : Hexadecimal RapidIO address of inbound window for both nodes'
	echo $'             Default is ' $DFLT_IBA_ADDR
    echo $'SKT_PREFIX : First 3 digits of 4 digit socket numbers'
	echo $'             Default is ' $DFLT_SKT_PREFIX
    echo $'DMA_SYNC2  : 0 - blocking, 1 - async, 2 - fire and forget'
    echo $'             Default is same as DMA_SYNC'
    echo $'DMA_SYNC3  : 0 - blocking, 1 - async, 2 - fire and forget'
    echo $'             Default is same as DMA_SYNC'
	exit 1
fi;

# ensure hex values are correctly prefixed
if [[ $IBA_ADDR != 0x* ]] && [[ $IBA_ADDR != 0X* ]]; then
        IBA_ADDR=0x$IBA_ADDR
fi

INTERP_TRANS=(NW SW NW_R SW_R NW_R_ALL DEFAULT);
INTERP_SYNC=(BLOCK ASYNC FAF DEFAULT);

echo $'\nStarting regression:\n'
echo 'MAST       :' $MAST_NODE
echo 'SLAVE      :' $SLAVE_NODE
echo 'MAST_MPNUM :' $MAST_MPNUM
echo 'SLAVE_MPNUM:' $SLAVE_MPNUM
echo 'DIR        :' $HOMEDIR
echo 'WAIT TIME  :' $WAIT_TIME ' SECONDS'
echo 'TRANS      :' $TRANS ${INTERP_TRANS[TRANS_IN]}
echo 'SYNC       :' $SYNC_IN  ${INTERP_SYNC[SYNC_IN]}
echo 'SYNC2      :' $SYNC2_IN ${INTERP_SYNC[SYNC2_IN]}
echo 'SYNC3      :' $SYNC3_IN ${INTERP_SYNC[SYNC3_IN]}
echo 'IBA_ADDR   :' $IBA_ADDR
echo 'SKT_PREFIX :' $SKT_PREFIX

if [ "$TRANS_IN" == "5" ]; then
	DMA_TRANS=( 0 )
else
	DMA_TRANS=( "$TRANS_IN" )
fi

if [ "$SYNC_IN" == "3" ]; then
	DMA_SYNC=( 0 )
else
	DMA_SYNC=( "${SYNC_IN}" )
	if [ "$SYNC2_IN" == "3" ]; then
		SYNC2=${SYNC_IN}
	else
		SYNC2=${SYNC2_IN}
	fi
	if [ "$SYNC3_IN" == "3" ]; then
		SYNC3=${SYNC_IN}
	else
		SYNC3=${SYNC3_IN}
	fi
fi

# Before proceeding further, recompile goodput 
# and demonstrate that each can execute the startup script

NODES=( "$MAST_NODE" "$SLAVE_NODE" )
MPNUM=( "$MAST_MPNUM" "$SLAVE_MPNUM" )
declare -a DESTIDS
IDX=0

for node in ${NODES[@]}; do 
	if ( ssh -T root@"$node" '[ -d ' $HOMEDIR ' ]' ); then
		echo $' '
		echo $node " Directory $HOMEDIR exists!"
	else
		echo $node " Directory $HOMEDIR does not exist!!! exiting..."
		exit 1
	fi

	echo $node " Building goodput."
	ssh -T root@"$node" <<BUILD_SCRIPT   
cd $HOMEDIR
make -s clean
make -s all
BUILD_SCRIPT

	if ( ! ssh -T root@"$node" test -f $HOMEDIR/goodput ); then
		echo $'\n'
		echo $node " $HOMEDIR/goodput could not be built. Exiting..."
		exit 1
	fi

	echo $node " STARTING GOODPUT"

	LOGNAME=mport${MPNUM[${IDX}]}'/start_target.log'
	LOG_FILE_DIR=${HOMEDIR}/logs/mport${MPNUM[${IDX}]}

	ssh -T root@"$node" <<NODE_START
rm -f ${LOG_FILE_DIR}/*.log
rm -f ${LOG_FILE_DIR}/*.res
rm -f ${LOG_FILE_DIR}/*.out

cd ${HOMEDIR}/scripts
./create_start_scripts.sh ${MPNUM[${IDX}]} ${SKT_PREFIX} ${IBA_ADDR}

cd ..

echo $node " Quitting out of OLD goodput screen session, if one exists"
screen -S goodput -p 0 -X stuff $'quit\r'

sleep 10

screen -dmS goodput ./goodput
screen -S goodput -p 0 -X stuff $'log logs/${LOGNAME}\r'
screen -S goodput -p 0 -X stuff $'. start_target\r'
screen -S goodput -p 0 -X stuff $'close\r'

sleep 1
NODE_START

	if ( ! ssh -T root@"$node" test -s $HOMEDIR/logs/${LOGNAME} ); then
		echo $'\n'
		echo "$node $HOMEDIR/logs/${LOGNAME} check failed. Exiting..."
		exit 1
	fi

	DESTIDS[${IDX}]=$(ssh -T root@${node} 'grep -E dest_id '${HOMEDIR}/logs/${LOGNAME} | awk {$'print $5\r'})

	echo $node " Destination ID is " ${DESTIDS[${IDX}]}
	echo $' '
	IDX=${IDX}+1
done

LABEL_LOG=${HOMEDIR}/logs/mport${MAST_MPNUM}/label.res

ssh -T root@${MAST_NODE} << LOG_FILE_SLAVE 
echo 'Test run started ' "$(eval date)" > $LABEL_LOG
echo 'GENERATING SCRIPTS ON SLAVE ${SLAVE_NODE}, PARAMETERS ARE' >> $LABEL_LOG
echo 'WAIT TIME  :' $WAIT_TIME ' SECONDS' >> $LABEL_LOG
echo 'TRANS      : 0 ' ${INTERP_TRANS[0]} >> $LABEL_LOG
echo 'SYNC       : 0 ' ${INTERP_SYNC[0]} >> $LABEL_LOG
echo 'SYNC2      : 0 ' ${INTERP_SYNC[0]} >> $LABEL_LOG
echo 'SYNC3      : 0 ' ${INTERP_SYNC[0]} >> $LABEL_LOG
echo 'DID        :' ${DESTIDS[0]} >> $LABEL_LOG
echo 'IBA_ADDR   : ' $IBA_ADDR >> $LABEL_LOG
echo 'SKT_PREFIX : ' $SKT_PREFIX >> $LABEL_LOG
echo 'MPORT_NUM  : ' $SLAVE_MPNUM >> $LABEL_LOG
LOG_FILE_SLAVE

ssh -T root@${MAST_NODE} cat ${LABEL_LOG}

# NOTE: DESTIDS[0] below is correct, because the slave needs to know
#       the destination ID of the master.

ssh -T root@${SLAVE_NODE} > /dev/null <<SLAVE_SCRIPT_GEN
cd $HOMEDIR/scripts
./create_perf_scripts.sh $WAIT_TIME 0 0 ${DESTIDS[0]} $IBA_ADDR $SKT_PREFIX ${SLAVE_MPNUM} 0 0
SLAVE_SCRIPT_GEN

## DMA_LAT_SZ=( 
OBWIN_LAT_SZ=( 1B 2B 4B 8B )
OBWIN_LAT_PREFIX=ol

DMA_LAT_SZ=(1B 2B 4B 8B 16B 32B 64B 128B 256B 512B
	1K 2K 4K 8K 16K 32K 64K 128K 256K 512K
	1M 2M 4M)
DMA_LAT_PREFIX=dl

# NOTE: The original intent of the script was to loop through all
#       combinations of TRANS and SYNC.  This is just not worthwhile,
#       as there's no way to check the logs.  As a result, the
#       current script operates on just one TRANS and SYNC.

for TRANS in ${DMA_TRANS[@]}; do
	for SYNC in ${DMA_SYNC[@]}; do
		if [ ! $SYNC_IN == 3 ]; then
			if [ "$SYNC2_IN" == "3" ]; then
				SYNC2=$SYNC
			fi
			if [ "$SYNC3_IN" == "3" ]; then
				SYNC3=$SYNC
			fi
		fi
		LABEL_LOG=${HOMEDIR}/logs/mport${MAST_MPNUM}/label.res
		ssh -T root@${MAST_NODE} << LOG_FILE_MASTER 
echo 'GENERATED SCRIPTS ON MASTER '${MAST_NODE}' at '$(eval date) >> $LABEL_LOG
echo 'MASTER PARAMETERS ARE' >> $LABEL_LOG
echo 'WAIT TIME  :' $WAIT_TIME ' SECONDS' >> $LABEL_LOG
echo 'TRANS      :' $TRANS ${INTERP_TRANS[TRANS]} >> $LABEL_LOG
echo 'SYNC       :' $SYNC  ${INTERP_SYNC[SYNC]} >> $LABEL_LOG
echo 'SYNC2      :' $SYNC2 ${INTERP_SYNC[SYNC2]} >> $LABEL_LOG
echo 'SYNC3      :' $SYNC3 ${INTERP_SYNC[SYNC3]} >> $LABEL_LOG
echo 'DID        :' ${DESTIDS[1]} >> $LABEL_LOG
echo 'IBA_ADDR   : ' $IBA_ADDR >> $LABEL_LOG
echo 'SKT_PREFIX : ' $SKT_PREFIX >> $LABEL_LOG
echo 'MPORT_NUM  : ' $MAST_MPNUM >> $LABEL_LOG
LOG_FILE_MASTER

		ssh -T root@${MAST_NODE} tail --lines=11 ${LABEL_LOG}

		echo 'EXECUTING ' ${HOMEDIR}/mport${MAST_MPNUM}/run_all_perf
		let "TOT_WAIT = ((450 * ($WAIT_TIME + 1)) / 60) + 1"
		echo 'ESTIMATING ' $TOT_WAIT ' MINUTES TO COMPLETION...'

		LOG_FILE_NAME=${HOMEDIR}/logs/mport${MAST_MPNUM}/run_all_perf_done.log

		ssh -T root@${MAST_NODE} > /dev/null << MAST_SCRIPT_RUN
rm -f ${LOG_FILE_NAME}
cd $HOMEDIR/scripts
# NOTE: DESTIDS[1] below is correct, because the master needs to know
#       the destination ID of the slave
./create_perf_scripts.sh $WAIT_TIME $TRANS $SYNC ${DESTIDS[1]} $IBA_ADDR $SKT_PREFIX $MAST_MPNUM $SYNC2 $SYNC3
screen -S goodput -p 0 -X stuff $'scrp mport${MAST_MPNUM}\r'
screen -S goodput -p 0 -X stuff $'. run_all_perf\r'
MAST_SCRIPT_RUN

		while ( ! ssh -T root@"${MAST_NODE}" test -s ${LOG_FILE_NAME}) 
		do
			sleep 60
			let "TOT_WAIT = $TOT_WAIT - 1"
			echo 'NOW ' $TOT_WAIT ' MINUTES TO COMPLETION...'
		done

		let "LONG_WAIT= $WAIT_TIME * 2"

		SUBDIR=mport${MAST_MPNUM}/obwin_lat
		echo 'EXECUTING ALL SCRIPTS IN ' ${HOMEDIR}'/'${SUBDIR}

		LOGNAME=mport${MAST_MPNUM}/obwin_lat_write.log
		ssh -T root@${MAST_NODE} << MAST_OBWIN_LAT_ST
screen -S goodput -p 0 -X stuff $'log logs/${LOGNAME}\r'
MAST_OBWIN_LAT_ST

		for SZ in ${OBWIN_LAT_SZ[@]}; do
			DONE_LOGNAME=${HOMEDIR}/logs/mport${SLAVE_MPNUM}/OBWIN_RX${SZ}DONE.log
			# Run slave target loop for write latency test 
			SCRIPTNAME='olT'${SZ}'.txt'
			echo ${SLAVE_NODE} ${SCRIPTNAME}
			ssh -T root@"${SLAVE_NODE}"  << SLAVE_OBWIN_LAT_WR
rm -f ${DONE_LOGNAME}
screen -S goodput -p 0 -X stuff $'scrp ${SUBDIR}\r'
screen -S goodput -p 0 -X stuff $'. ${SCRIPTNAME}\r'
SLAVE_OBWIN_LAT_WR
			while ( ! ssh -T root@"${SLAVE_NODE}" test -s ${DONE_LOGNAME}) 
			do
				sleep $WAIT_TIME
			done

			# Run master loop for write latency test 
			SCRIPTNAME='olW'${SZ}'.txt'
			echo ${MAST_NODE} ${SCRIPTNAME}
			ssh -T root@"${MAST_NODE}"  << MAST_OBWIN_LAT_WR
screen -S goodput -p 0 -X stuff $'scrp ${SUBDIR}\r'
screen -S goodput -p 0 -X stuff $'. ${SCRIPTNAME}\r'
MAST_OBWIN_LAT_WR
			sleep $LONG_WAIT
		done

		ssh -T root@${MAST_NODE} << MAST_OBWIN_LAT_ST
screen -S goodput -p 0 -X stuff $'close\r'
MAST_OBWIN_LAT_ST

		SUBDIR=mport${MAST_MPNUM}/dma_lat
		echo 'EXECUTING ALL SCRIPTS IN ' ${HOMEDIR}'/'${SUBDIR}

		LOGNAME=mport${MAST_MPNUM}/dma_lat_write.log
		ssh -T root@${MAST_NODE} << MAST_DMA_LAT_ST
screen -S goodput -p 0 -X stuff $'log logs/${LOGNAME}\r'
MAST_DMA_LAT_ST

		ssh -T root@${SLAVE_NODE} << SLAVE_DMA_LAT_ST
screen -S goodput -p 0 -X stuff $'log logs/${LOGNAME}\r'
SLAVE_DMA_LAT_ST

		for SZ in ${DMA_LAT_SZ[@]}; do
			# Run slave target loop for write latency test 
			SCRIPTNAME='dlT'${SZ}'.txt'
			SUBDIR=mport${SLAVE_MPNUM}/dma_lat
			echo ${SLAVE_NODE} ${SCRIPTNAME}
			ssh -T root@"${SLAVE_NODE}"  << SLAVE_DMA_LAT_WR
screen -S goodput -p 0 -X stuff $'scrp ${SUBDIR}\r'
screen -S goodput -p 0 -X stuff $'. ${SCRIPTNAME}\r'
SLAVE_DMA_LAT_WR
			sleep 2
			
			# Run master loop for write latency test 
			SCRIPTNAME='dlW'${SZ}'.txt'
			SUBDIR=mport${MAST_MPNUM}/dma_lat
			echo ${MAST_NODE} ${SCRIPTNAME}
			ssh -T root@"${MAST_NODE}"  << MAST_DMA_LAT_WR
screen -S goodput -p 0 -X stuff $'scrp ${SUBDIR}\r'
screen -S goodput -p 0 -X stuff $'. ${SCRIPTNAME}\r'
MAST_DMA_LAT_WR
			sleep ${LONG_WAIT}

			ssh -T root@"${MAST_NODE}"  << MAST_DMA_LAT_END
screen -S goodput -p 0 -X stuff $'kill 0\r'
screen -S goodput -p 0 -X stuff $'wait 0 d\r'
MAST_DMA_LAT_END
			sleep 2
		done

		ssh -T root@${MAST_NODE} << MAST_DMA_LAT_ST
screen -S goodput -p 0 -X stuff $'close\r'
echo 'Test run finished ' "$(eval date)" >> $LABEL_LOG
MAST_DMA_LAT_ST

		ssh -T root@${SLAVE_NODE} << SLAVE_DMA_LAT_ST
screen -S goodput -p 0 -X stuff $'close\r'
SLAVE_DMA_LAT_ST
		
		# Now start analyzing/checking output.
		
		ssh -T root@${MAST_NODE} << MAST_LOGS_CHECK
cd ${HOMEDIR}/logs/mport${MAST_MPNUM}
./summ_thru_logs.sh > all_thru.res
./summ_lat_logs.sh > all_lat.res
./check_thru_logs.sh all_thru.res
./check_lat_logs.sh all_lat.res
MAST_LOGS_CHECK

		# If there was a failure, keep the goodput sessions around
		# for debug purposes.
		if [ -s ${HOMEDIR}/logs/mport${MAST_MPNUM}/thru_fail.out ]; then
			exit;
		fi;

		if [ -s ${HOMEDIR}/logs/mport${MAST_MPNUM}/lat_fail.out ]; then
			exit;
		fi;

	done
done

for node in ${NODES[@]}; do
	ssh -T root@"$node" <<NODE_STOP
cd ${HOMEDIR}/scripts
echo $node " Stopping goodput sessions, if they exist."
screen -S goodput -p 0 -X stuff $'quit\r'
sleep 10
NODE_STOP

exit
