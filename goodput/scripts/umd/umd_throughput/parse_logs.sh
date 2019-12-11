#!/bin/sh

if [ $# -lt 1 ]; then
    results_file="results.csv"
fi

rm -f $results_file
echo "Threads,DMA_Engines,Size,Throughput (Gbps)" > $results_file

for log in `ls *.log`; do
    regexp="(d1_)([0-9]+[BKM])([0-9])(T)([0-9])(D\\.log)"
    size=`echo $log | sed -E "s/${regexp}/\2/"`
    threads=`echo $log | sed -E "s/${regexp}/\3/"`
    dma=`echo $log | sed -E "s/${regexp}/\5/"`
    total_line=`grep Total $log`
    gbps=`echo $total_line | awk '{print $4}'`

    echo "${threads},${dma},${size},${gbps}" >> $results_file

    echo "Threads ${threads} DMA_Engines ${dma} Size ${size} Gbps ${gbps}"
done
