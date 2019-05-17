#!/bin/bash
# usage: ./make_install.sh <server> <rootPath> <memSize> <unixGroup> <USERID>

# Common to both master and slave
#
INSTALL_ROOT="/opt/rapidio/.install"
. $INSTALL_ROOT/script/make_install_common.sh $1 $2 $3 $4 $5
USERID=$5

# Install scripts
#
echo "Installing start scripts..."
SCRIPT_FILES=( rio_start.sh stop_rio.sh all_start.sh stop_all.sh check_all.sh 
    rsock0_start.sh rsock0_stop.sh
    all_down.sh )

for f in "${SCRIPT_FILES[@]}"
do
    cp $SCRIPTS_PATH/$f $SOURCE_PATH/$f
done


# Install fmd configuration
# format of input file: <master|slave> <hostname> <rioname> <nodenumber>
#
echo "Installing fmd configuration..."
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
