#!/usr/bin/python3

import os

debug = False


def summarize_thruput_log(fname):
    """Summarize thruput log file
    """
    outList    = []
    outfname = fname + ".res"
    outList.append("Processing throughput log file: {f}".format(f=fname))
    outList.append("Output filename is:             {f}".format(f=outfname))

    with open(fname, 'r') as f:
        fdataList = f.readlines()
        echo=None
        total    = None
        xfersize = None
        availCpu = None
        ssdist   = None
        sssize   = None
        dsdist   = None
        dssize   = None
        hdrPrinted = False

        for index, line in enumerate(fdataList):
            if "Kernel" in line:
                kernelLine = fdataList[index + 1].split()
                try:
                    availCpu, procUser, procKern, cpuOcc = kernelLine
                except ValueError:
                    print("Error in Kernel record - expecting 4 fields got {len}: {l}".format(len=len(kernelLine), l=kernelLine)) 
                    raise
                if debug: print("Kernel: {avail} {pu} {pk} {cpu}".format(avail=availCpu, pu=procUser, pk=procKern, cpu=cpuOcc))
            if "Total" in line and echo is not None:
                # only pick off the "Total" line after the 'echo' line has been found
                try:
                    total, data, mbps, gbps, _msgs, linkOcc = line.split()
                except ValueError:
                    print("Error in Total line - expecting 6 fields got {len}: {l}".format(l=line, len=len(line.split())))
                if debug: print("Total: {t} {data} {mbps} {gbps} {linkOcc}".format(t=total, data=data, mbps=mbps, gbps=gbps, linkOcc=linkOcc))
            if "echo" in line:
                try:
                    echo, _dma, _throughput, blksize, xfersize, _other = line.split()
                except ValueError:
                    try:
                        echo, _dma, _throughput, blksize, xfersize, _other, ssdist, sssize, dsdist, dssize = line.split()
                    except ValueError:
                        print("Error in echo line - expecting 6 or 10 fields got {len}: {l}".format(l=line, len=len(line.split())))
                if debug: print("Echo: {blk} {xfer}".format(blk=blksize, xfer=xfersize))
            if total is not None and xfersize is not None and availCpu is not None:
                if not hdrPrinted:
                    hdr = "{xfer:>8} {mbps:>8} {gbps:>8} {linkOcc:>8} {availCpu:>8} {pu:>8} {pk:>8} {cpuOcc:>8}".format(xfer="SIZE", mbps="Mbps", gbps="Gbps", linkOcc="LinkOcc", availCpu="AvailCpu", pu="UserCPU", pk="KernCpu", cpuOcc="CPU OCC %")
                    if ssdist is not None:
                        hdr += "{ssdist:>8} {sssize:>8} {dsdist:>8} {dssize:>8}".format(ssdist="ssdist", sssize="sssize", dsdist="dsdist", dssize="dssize")
                    outList.append(hdr)
                    hdrPrinted = True

                outLine = "{xfer:>8} {mbps:>8} {gbps:>8} {linkOcc:>8} {availCpu:>8} {pu:>8} {pk:>8} {cpuOcc:>8}".format(xfer=xfersize, mbps=mbps, gbps=gbps, linkOcc=linkOcc, availCpu=availCpu, pu=procUser, pk=procKern, cpuOcc=cpuOcc)
                if ssdist is not None:
                    outLine += " {ssdist:>8} {sssize:>8} {dsdist:>8} {dssize:>8}".format(ssdist=ssdist, sssize=sssize, dsdist=dsdist, dssize=dssize)
                outList.append(outLine)
                echo = None
                xfersize = None
                availCpu = None

        outList.append("")

        with open(outfname, 'w') as f:
            for l in outList:
                print(l)
                f.write("{l}\n".format(l=l))

print("Scanning directory for log files")
for dirname, dirnames, filenames in os.walk('.'):
    for filename in filenames:
        if os.path.splitext(filename)[1] in [".log"]:
            try:
                summarize_thruput_log(os.path.join(dirname, filename))
            except ValueError:
                # should be save to ignore ValueError messages
                pass

