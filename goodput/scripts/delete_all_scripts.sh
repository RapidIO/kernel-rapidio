#!/bin/bash
MPORT=0

if [ -n "$1" ]; then
        MPORT=$1
else
        echo $'\nScript deletes all performance scripts for an mport.'
        echo $'mport is a required parameter.'
	exit
fi

echo $'Deleting all performance scripts for mport' ${MPORT}

cd ../mport${MPORT}

rm -f dma_thru/d1*
rm -f dma_thru_read dma_thru_write

rm -f udma_thru/udma*
rm -f udma_thru_read udma_thru_write

rm -f dma_lat/dl*
rm -f dma_lat_read

rm -f udma_lat/udl*
rm -f udma_lat_read

rm -f pdma_thru/pd*
rm -f pdma_thru_*_read pdma_thru_*_write

rm -f obwin_lat/ol*
rm -f obwin_lat_read

rm -f obwin_thru/o1R*
rm -f obwin_thru/o1W*
rm -f obwin_thru/o8R*
rm -f obwin_thru/o8W*
rm -f obwin_thru_read obwin_thru_write

rm -f msg_thru/m*
rm -f msg_thru_tx

rm -f umsg_thru/m*
rm -f umsg_thru_tx

rm -f msg_lat/m*
rm -f msg_lat_tx

rm -f umsg_lat/m*.txt
rm -f umsg_lat_read


cd ..
rm -f start_source start_target
