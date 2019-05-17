#!/bin/bash
# usage: ./make_install.sh <server> <rootPath> <memSize> <unixGroup>

# Common to both master and slave
#
INSTALL_ROOT="/opt/rapidio/.install"
. $INSTALL_ROOT/script/make_install_common.sh $1 $2 $3 $4


# Set ownership of files
#
cd $SOURCE_PATH/..
chown -R root.$GRP rapidio_sw

cd $CONFIG_PATH/..
chown -R root.$GRP rapidio
