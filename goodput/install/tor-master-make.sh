#!/bin/bash
# usage: ./make_install.sh <server> <rootPath> <memSize> <unixGroup> <USERID>

# Common to both master and slave
#
INSTALL_ROOT="/opt/rapidio/.install"
. $INSTALL_ROOT/script/make_install_common.sh $1 $2 $3 $4 $5
USERID=$5

result=$?
if [ $result -ne 0 ]
then
	echo Failed, exiting...
	exit $result
fi

# Install scripts
#
echo "Installing start scripts..."
SCRIPT_FILES=( rio_start.sh stop_rio.sh all_start.sh stop_all.sh check_all.sh 
    rsock0_start.sh rsock0_stop.sh
    all_down.sh )

for f in "${SCRIPT_FILES[@]}"
do
    cp $SCRIPTS_PATH/$f $SOURCE_PATH/$f
    result=$?
	if [ $result -ne 0 ]
	then
		echo Copy failed, exiting...
		exit $result
	fi
done


# Install fmd configuration
# format of input file: <master|slave> <hostname> <rioname> <nodenumber>
#
echo "Installing fmd configuration..."
# Get list of node names for each node in the install node_list
# and determine node number of the master node
HOSTL=''
MAST_LABEL='master'
MAST_NODE=''
while read -r line || [[ -n "$line" ]]; do
    arr=($line)
    f=${arr[3]}
    HOSTL="$HOSTL -vH_${f^^}=${arr[1]}"

# NOTE: Node list has already been checked for duplicated/missing master node entries
    if [ ${arr[0]} = "$MAST_LABEL" ]
    then
	MAST_NODE=${arr[3]}
    fi
done < "$INSTALL_ROOT/$NODEDATA_FILE"

# Figure out node number of master node
if [ -z $MAST_NODE ]
then
	echo "Master node not defined, failed install.  Exiting..."
	exit 1
fi

# Determine destination ID of the master node
# line syntax:
# ENDPOINT <ep_name> PORT <#> <ct> <pw> <pw> <ls> <IDLE1|2|3> <EM_ON|EM_OFF> {<dev08|dev16|dev32> <devID> <hc>} END PEND
MAST_DESTID=$(grep -e ENDPOINT < $INSTALL_ROOT/$TMPL_FILE | grep "\b${MAST_NODE}\b" | while read ep_kw epname port_kw port_num ct pw_max pw_cfg pt_ls idle em_ctl did_sz did unused; do echo $did; done)

if [ -z $MAST_DESTID ]
then
	echo "Master destID not found, failed install.  Exiting..."
	exit 1
fi

HOSTL="$HOSTL -vMAST_DID=${MAST_DESTID}"

awk -vM=$MEM_SIZE $HOSTL '
    /MEMSZ/{gsub(/MEMSZ/, M);}
    /node32/{if(H_NODE32 != "") {gsub(/node32/, H_NODE32);} else {$0="";}}
    /node31/{if(H_NODE31 != "") {gsub(/node31/, H_NODE31);} else {$0="";}}
    /node30/{if(H_NODE30 != "") {gsub(/node30/, H_NODE30);} else {$0="";}}
    /node29/{if(H_NODE29 != "") {gsub(/node29/, H_NODE29);} else {$0="";}}
    /node28/{if(H_NODE28 != "") {gsub(/node28/, H_NODE28);} else {$0="";}}
    /node27/{if(H_NODE27 != "") {gsub(/node27/, H_NODE27);} else {$0="";}}
    /node26/{if(H_NODE26 != "") {gsub(/node26/, H_NODE26);} else {$0="";}}
    /node25/{if(H_NODE25 != "") {gsub(/node25/, H_NODE25);} else {$0="";}}
    /node24/{if(H_NODE24 != "") {gsub(/node24/, H_NODE24);} else {$0="";}}
    /node23/{if(H_NODE23 != "") {gsub(/node23/, H_NODE23);} else {$0="";}}
    /node22/{if(H_NODE22 != "") {gsub(/node22/, H_NODE22);} else {$0="";}}
    /node21/{if(H_NODE21 != "") {gsub(/node21/, H_NODE21);} else {$0="";}}
    /node20/{if(H_NODE20 != "") {gsub(/node20/, H_NODE20);} else {$0="";}}
    /node19/{if(H_NODE19 != "") {gsub(/node19/, H_NODE19);} else {$0="";}}
    /node18/{if(H_NODE18 != "") {gsub(/node18/, H_NODE18);} else {$0="";}}
    /node17/{if(H_NODE17 != "") {gsub(/node17/, H_NODE17);} else {$0="";}}
    /node16/{if(H_NODE16 != "") {gsub(/node16/, H_NODE16);} else {$0="";}}
    /node15/{if(H_NODE15 != "") {gsub(/node15/, H_NODE15);} else {$0="";}}
    /node14/{if(H_NODE14 != "") {gsub(/node14/, H_NODE14);} else {$0="";}}
    /node13/{if(H_NODE13 != "") {gsub(/node13/, H_NODE13);} else {$0="";}}
    /node12/{if(H_NODE12 != "") {gsub(/node12/, H_NODE12);} else {$0="";}}
    /node11/{if(H_NODE11 != "") {gsub(/node11/, H_NODE11);} else {$0="";}}
    /node10/{if(H_NODE10 != "") {gsub(/node10/, H_NODE10);} else {$0="";}}
    /node9/{if(H_NODE9 != "") {gsub(/node9/, H_NODE9);} else {$0="";}}
    /node8/{if(H_NODE8 != "") {gsub(/node8/, H_NODE8);} else {$0="";}}
    /node7/{if(H_NODE7 != "") {gsub(/node7/, H_NODE7);} else {$0="";}}
    /node6/{if(H_NODE6 != "") {gsub(/node6/, H_NODE6);} else {$0="";}}
    /node5/{if(H_NODE5 != "") {gsub(/node5/, H_NODE5);} else {$0="";}}
    /node4/{if(H_NODE4 != "") {gsub(/node4/, H_NODE4);} else {$0="";}}
    /node3/{if(H_NODE3 != "") {gsub(/node3/, H_NODE3);} else {$0="";}}
    /node2/{if(H_NODE2 != "") {gsub(/node2/, H_NODE2);} else {$0="";}}
    /node1/{if(H_NODE1 != "") {gsub(/node1/, H_NODE1);} else {$0="";}}
    {gsub(/MAST_DID/, MAST_DID);}
    {print}' $INSTALL_ROOT/$TMPL_FILE > $CONFIG_FILE


# Set ownership of files
#
cd $SOURCE_PATH/..
chown -R $USERID.$GRP rapidio_sw

cd $CONFIG_PATH/..
chown -R $USERID.$GRP rapidio
