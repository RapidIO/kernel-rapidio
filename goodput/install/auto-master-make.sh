#!/bin/bash
# usage: ./make_install.sh <server> <rootPath> <memSize> <unixGroup> <USERID>

# Common to both master and slave
#
INSTALL_ROOT="/opt/rapidio/.install"
. $INSTALL_ROOT/script/make_install_common.sh $1 $2 $3 $4 $5
USERID=$5

# Set ownership of files
#
cd $SOURCE_PATH/..
chown -R $USERID.$GRP kernel_rapidio

cd $CONFIG_PATH/..
chown -R $USERID.$GRP rapidio
