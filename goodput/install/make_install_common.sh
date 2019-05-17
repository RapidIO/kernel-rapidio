#!/bin/bash
# usage: ./make_install_common.sh <server> <rootPath> <memSize> <unixGroup> <USERID>

# Setup passed in parameters
#
SERVER=$1
ROOT_PATH=$2
MEM_SIZE=$3
GRP=$4
USERID=$5

# Files required by install, match those names in install.sh
#
NODEDATA_FILE="nodeData.txt"
SRC_TAR="kernel_rapidio.tar"
TMPL_FILE="config.tmpl"

echo "Installing on $(hostname)..."

# Setup install location
#
INSTALL_ROOT="/opt/rapidio/.install"
SOURCE_PATH="/opt/rapidio/kernel_rapidio"
CONFIG_PATH="/etc/rapidio"
SCRIPTS_PATH=$SOURCE_PATH"/install"


# Ensure clean installs
# (Do not create/clean $INSTALL_ROOT, that was done for us by install.sh)
#
rm -rf $SOURCE_PATH;mkdir -p $SOURCE_PATH
rm -rf $CONFIG_PATH;mkdir -p $CONFIG_PATH

# Create directories
#
mkdir -p /var/tmp/rapidio
chmod 777 /var/tmp/rapidio
chown -R $USERID.$GRP /var/tmp/rapidio

# Get the source files from the central server
#
echo "Transferring install files from server $SERVER..."
scp $USERID@"$SERVER":$ROOT_PATH/* $INSTALL_ROOT/ > /dev/null
chown -R $USERID.$GRP $INSTALL_ROOT


cd $SOURCE_PATH
result=$?
if [ $result -ne 0 ]
then
	echo cd failed, exiting...
	exit $result
fi

tar -xomf $INSTALL_ROOT/$SRC_TAR > /dev/null
result=$?
if [ $result -ne 0 ]
then
	echo tar failed, exiting...
	exit $result
fi

# Compile the source
#

# Install kernel driver (rio_mport_cdev, rio_cm, rio_scan)
echo "Compile rio_mport_cdev sources..."
make -s clean
make -s all
result=$?
if [ $result -ne 0 ]
then
	echo Driver: Make of installed code failed, exiting...
	exit $result
fi
make -s install
result=$?
if [ $result -ne 0 ]
then
	echo Driver: Install failed, exiting...
	exit $result
fi

# Install kernel driver (rio_mport_cdev, rio_cm, rio_scan)
echo "Compile rio_mport_lib sources..."
cd libmport
make -s clean
make -s all
result=$?
if [ $result -ne 0 ]
then
	echo LIBMPORT: Make of installed code failed, exiting...
	exit $result
fi

# Install kernel driver (rio_mport_cdev, rio_cm, rio_scan)
echo "Compile goodput sources..."
cd ../goodput
make -s clean
make -s all
result=$?
if [ $result -ne 0 ]
then
	echo GOODPUT: Make of installed code failed, exiting...
	exit $result
fi

# Create scripts required for DMA TUN and others...
#
echo "Creating goodput scripts..."
cd scripts
./create_start_scripts.sh 0 234 0x200000000


# Remove old configuration file
#
rm -f $CONFIG_FILE
