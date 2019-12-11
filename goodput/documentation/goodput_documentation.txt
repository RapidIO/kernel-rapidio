/*! \page goodput_docs Goodput Introduction
 *
 * \section intro_sec Introduction
 * The goodput tool measures goodput (actual data transferred) and latency for 
 * the rapidio_sw/common/libmport direct I/O, DMA, and Messaging interfaces.
 *
 * \section fast_start_sec Getting Started
 *
 * To start the systems RapidIO interfaces, complete the
 * following steps:
 *
 * -# Power up the system switch, if one exists in the system.
 * -# Power up all slave nodes.
 *    Wait until the Linux interface is displayed on the terminal.
 * -# Power up the master ("enumerator") node.
 *    Wait until the Linux interface is displayed on the terminal.
 * -# Log in as guest on each node.
 * -# Create terminal sessions on each node.
 * -# On the master node, enter the following command in the terminal:
 *    "/opt/rapidio/rapidio_sw/rio_start.sh".
 * 
 * The goodput tools are installed at 
 * /opt/rapidio/rapidio_sw/utils/goodput.  
 * 
 * \subsection goodput_exec_sec Goodput Quick Start
 *
 * The Goodput and Ugoodput tools support a rich set of performance measurement
 * capabilities.  Details of these capabilities are found later in this
 * documentation.
 *
 * A standard set of goodput measurements can be performed by executing the 
 * goodput/test/regression.sh script.  The script uses two nodes to 
 * perform all measurements,
 * summarizes those measurements, and checks for errors.
 *
 * The two nodes must
 * meet the following system requirements:
 *
 * - ssh been configured to allow root access from the node executing 
 *   the regression.sh script to the two nodes 
 * - The 'screen' utility has been installed on the two nodes
 * - The RRMAP software package has been installed using the same directory
 *   path on the two nodes 
 * 
 * The regression.sh script accepts the following parameters:
 *
 * - MAST       : Name of master node/IP address
 * - SLAVE      : Name of slave node/IP address
 * - MAST_MPNUM : Master node mport number (usually 0)
 * - SLAVE_MPNUM: Slave node mport number (usually 0)
 *
 * All parameters below are optional.  Default values are shown.
 *
 * - DIR        : Directory on both MAST and SLAVE to run tests.
 *                Default is the absolute directory of test/regression.sh
 *                on the node executing test/regression.sh.
 *                If this same directory does not exist on both MAST and SLAVE,
 *                it is not possible to run regression.sh!
 * - WAIT       : Time in seconds to wait before perf measurement
 *                Default is  30.  Each second requires about 8 minutes of 
 *                execution time i.e. 30 seconds of individual measurements
 *                result in 4 hours of script execution time.
 * - DMA_TRANS  : DMA write transaction type
 *                0 NW, 1 SW, 2 NW_R, 3 SW_R 4 NW_R_ALL
 *                0 is the default.
 * - DMA_SYNC   : 0 - blocking, 1 - async, 2 - fire-and-forget
 *                Blocking: A DMA transaction must complete before the next
 *                DMA transaction will be started.
 *                Async: Completion of the DMA transaction is checked
 *                separately.  Similar performance to Blocking.  
 *                Fire-and-forget: DMA transaction completion is not checked.
 *                NOTE: Fire-and-forget can result in a "burst" of
 *                transactions at the start as the DMA transaction queue is
 *                filled.  
 * - IBA_ADDR   : RapidIO address of inbound window for both nodes
 *                Default is hexadecimal 200000000
 * - SKT_PREFIX : First 3 digits of 4 digit socket numbers
 *                Default is  234.
 *                Used for messaging measurements.
 * - DMA_SYNC2  : 0 - blocking, 1 - async, 2 - fire and forget
 *                Default is same as DMA_SYNC
 * - DMA_SYNC3  : 0 - blocking, 1 - async, 2 - fire and forget
 *                Default is same as DMA_SYNC
 *
 * All results are found on the MAST node, in the
 * goodput/logs/mport{MAST_MPNUM} directory.
 * This directory contains the following after a successfull test:
 * - label.res    : Information about regression parameters,
 *                  platform hardware, and execution time
 * - all_thru.res : Summary of all throughput measurement, as described in
 *                  \ref thru_sum_res.
 * - all_lat.res  : Summary of all latency measurements, as described in
 *                  \ref thru_sum_res.
 * - detailed measurement logs with names ending in ".log".
 *   For more information on the individual throughput measurements,
 *   refer to \ref goodput_cmd_overview_secn.
 *   For more information on the individual latency measurements,
 *   refer to \ref latency_cmd_overview_secn.
 * - results summary files, ending in ".res", for each detailed
 *   measurement log file.  For information on the latency and
 *   throughput results file data, refer to \ref lat_sum_res and
 *   \ref thru_sum_res.
 *
 * \subsubsection lat_sum_res Latency Summary Results
 *
 * Latency results are summarized using the following format:
 * <pre>
 * Processing latency log file :  obwin_lat_read.log
 * Output filename is          :  obwin_lat_read.log.res
 * SIZE     Transfers   Min_Usecs   Avg_Usecs   Max_Usecs
 *        1  15614082       1.779       1.864      31.432
 *        2  15694459       1.780       1.854     569.849
 *        4  15690140       1.781       1.854      76.678
 *        8  15690714       1.784       1.854     124.708
 * </pre>
 *
 * The columns have the following meaning:
 * - SIZE: Hexadecimal size of the transactions, powers of 2 from 1 up to 4 MB
 * - Transfers: Number of transfers used in the latency computation
 * - Min_Usecs: Minimum number of microseconds to complete one transfer
 * - Avg_Usecs: Average number of microseconds to complete one transfer
 * - Max_Usecs: Maximum number of microseconds to complete one transfer
 * Note that Max_Usecs is a measurement of operating system/scheduler latency.
 * Minimum and average latency figures indicate the usual performance for
 * each transaction.
 *
 * \subsubsection thru_sum_res Throughput Summary Results
 *
 * Throughput results are summarized using the following format:
 * <pre>
 * Processing througput log file :  pdma_thru_pdm_write.log
 * Output filename is            :  pdma_thru_pdm_write.log.res
 * SIZE        MBps     Gbps   LinkOcc  AvailCPU  UserCPU  KernCPU CPU OCC %
 *     4000  1547.073   12.377   13.028    10262       71     3217   128.16
 *     8000  1572.036   12.576   13.238    11225       52     1408    52.03
 *    10000  1585.969   12.688   13.356    11615       50      725    26.69
 *    20000  1591.853   12.735   13.405    11818       38      431    15.87
 *    40000  1573.115   12.585   13.247    11850       48      313    12.19
 *    80000  1562.327   12.499   13.156    11880       46      242     9.70
 *   100000  1580.099   12.641   13.306    11920       26      149     5.87
 *   200000  1592.371   12.739   13.409    11947       15       95     3.68
 *   400000  1596.779   12.774   13.447    11968        7       49     1.87
 * </pre>
 *
 * The columns have the following meaning:
 * - SIZE - Hexadecimal size of the transactions, powers of 2 from 1 up to 4 MB
 * - MBps - Goodput, payload data transferred.
 * - GBps - Goodput, MBps * 8
 * - LincOcc - Estimate of amount of bandwidth used on the link by packets
 * - AvailCPU: Ticks of processing time available in the measurement period,
 *             where each tick is 1/100th of a second.
 *             Each processor core contributes 100 ticks per second.
 * - UserCPU: Ticks spent in the measurement process (goodput)
 * - KernCPU - Ticks spent in the kernel
 * - CPU OCC % - Percentage of the CPU used by the measurement process.
 *
 * \subsection exec_sec Running Goodput
 * Goodput contains all functionality and commands to verify and measure kernel
 * mode applications.  
 * The goodput tool must be run as root.
 * To execute goodput, type "sudo ./goodput" while in the
 * "rapidio_sw/utils/goodput" directory.  This will start goodput using
 * the device /dev/rio_mport0.  To change to another mport, use "sudo ./goodput
 * {mport}, where mport is a digit from 0 to 9.
 *
 * \section cli_secn Command Line Interpreter Commands
 *
 * \subsection Common CLI Commands
 * Goodput integrates the "rapidio_sw/common/libcli" command line interpreter
 * library.  This library has the following base commands:
 *
 * \subsubsection help_secn Help Command
 *
 * The libcli help command is "?".  Type "?" to get a list of commands, or "?
 * <command>" to get detailed help for a command.
 *
 * \subsubsection debug_secn Debug Command
 *
 * Many commands have different levels of debug output.  The "debug" command
 * displays and optionally alters the debug output level.
 *
 * \subsubsection echo_secn Echo Command
 *
 * The "echo" command displays a copy of the text following the  command.  It is iseful for annotating
 * log files and scripts.
 *
 * \subsubsection log_secn Log Command
 *
 * The "log" command captures the input and output of a
 * goodput CLI session.
 *
 * \subsubsection close_secn Close Log Command
 *
 * The "close" command closes the log file opened with the "log"
 * command.
 *
 * \subsubsection quit_secn Quit Command
 *
 * The "quit" command exits cleanly, freeing up all RapidIO resources
 * that may be in use at the time by goodput.
 *
 * \subsubsection script_secn Script Command
 *
 * Libcli supports accepting input from script files.  The "script" command
 * specifies a file name to be used as the source of commands for the CLI
 * session.  Script files may call other script files.  Every script file is
 * run in its own CLI session -- environment changes (i.e. log file names)
 * in one script file do not affect the log file names in 
 * another. For information on selecting a directory of script 
 * files, see See \ref
 * scrpath_secn .  The goodput
 * command comes with many script files in the
 * "rapidio_sw/utils/goodput/scripts"  directory and subdirectories.
 *
 * Note: The '.' command is identical to the script command
 *
 * \subsubsection scrpath_secn Scrpath Command
 *
 * The "scrpath" command displays and optionally changes the 
 * directory path prepended to  script files
 * names.  Script files that do not begin with "/" or "\" have the prefix
 * prepended before the file is openned.
 *
 * \subsubsection set_cmd_secn Set Command
 *
 * The "set" command lists, sets, or clears CLI environment variables.
 * CLI environment variables can be used as the parameters to commands.
 * CLI environment variables are named "$var_name".
 *
 * \section threads_secn Goodput Thread Management
 * The goodput CLI manages 12 worker threads.
 * Worker threads are used to perform DMA, messaging, and direct I/O
 * accesses and measurements, as explained in \ref measurement_secn.  
 * The following sections explain the commands that manage worker threads.
 *
 * \subsection thread_secn Thread Command
 * The "thread" command starts a new thread.  Threads may be
 * required to run on a specific CPU, or may be allowed to run on any CPU.
 * Additionally, a thread may request its own private DMA engine, or may share 
 * a DMA engine with other threads.  For more information, see
 * \ref dma_meas_secn.
 *
 * A thread can be in one of following three states:
 * - Dead - The thread does not exist.
 * - Halted - The thread is waiting to accept a new command.
 * - Running - The thread is currently executing a command and cannot 
 * accept a new command until it stops running.
 *
 * Only a dead thread can be started with the thread command.
 * A running thread can be halted or killed.  A halted thread can be killed.
 * For more information, see \ref halt_secn and \ref kill_secn.
 *
 * \subsection halt_secn Halt Command
 * The "halt" command requests a thread to halt.  If the thread is currently
 * running a measurement, the thread wil halt and wait for the next command.
 *
 * \subsection kill_secn Kill Command
 * The "kill" command used to kill a thread, whatever its current state.  All resources
 * owned by the thread are released.  After a thread has been killed, the 
 * thread cannot process commands until it is restarted using the "thread"
 * command.  For more information, see \ref thread_secn.
 *
 * \subsection move_secn Move Command
 * If a thread is currently halted, the thread can be moved from one
 * cpu to another using the "move" command.  A moved thread retains all
 * allocated resources.
 *
 * \subsection isolcpu_secn IsolCPU Command
 * The isolcpu command checks the current Linux configuration for CPUs
 * which have been reserved for user specific tasks.  The isolcpu command
 * sets CLI environment variables named 'cpu1', 'cpu2', ..., 'cpuN' for 
 * each CPU found.  For more information on isolcpu configuration in Linux,
 * please research isolcpu Linux boot command line parameter.
 * 
 * NOTE: Isolcpus are not necessary to perform any measurements.  The accuracy
 * of some measurements is increased when they are executed on a CPU isolated 
 * from the Linux scheduler.
 *
 * \subsection stat_secn Status Command
 * The "status" command gives the current state of all threads.  Status has 
 * three variants, with General status as the default:
 *
 * - st i - Inbound window status. The command displays information about 
 * Direct I/O inbound window resources owned by the thread. 
 * - st m - Messaging status.  Displays information about messaging 
 * resources owned by the thread. 
 * - st - General status.  Displays information about the command that 
 * a thread is running/has run.
 *
 * \subsection wait_secn Wait Command
 * Wait until a thread reaches a particular state (dead, halted, or running).
 * The "wait" command is useful in scripts to ensure a command has started running successfully, and
 * that threads have halted/died before issuing another command.
 *
 * \section measurement_secn Goodput Measurements
 * Goodput measures goodput, RapidIO link occupancy and latency for 
 * Direct I/O, DMA, and messaging transactions.  
 * 
 * \subsection dio_dma_measurement_overview_secn Direct I/O and DMA  Configuration
 *
 * Direct I/O and DMA both produce read and write transactions
 * that access an inbound window on a target device.  A thread on node X must
 * be commanded to allocate an inbound window z, and a thread on node Y must be
 * commanded to perform direct I/O or DMA transactions to Node X inbound
 * window z. 
 *
 * The "IBAlloc" command requests that a thread allocate an inbound window.  
 *  Once the thread has finished inbound window allocation, the thread 
 * halts and can accept another command.  The location of the inbound window 
 * can be 
 * displayed using the "status i" command, as described in the \ref stat_secn.
 *
 * The "IBDealloc" command can be used 
 * to deallocate a previously allocated window.  
 *
 * The "dump" command dumps the
 * memory contents for an inbound window. 
 *
 * \subsection msg_measurement_overview_secn Messaging Configuration
 *
 * Messaging transactions
 * support socket style bind/listen/accept/connect semantics. 
 * For messaging measurements to occur, a thread on node
 * X must be commanded to receive on socket z, and a thread on node Y must be
 * commanded to send to node X socket z.
 *
 * \subsection destID_overview_secn Destination ID Configuration
 *
 * All commands that act as sources of transactions require a destination ID
 * for the receiver. To determine the destination ID of the receiver, execute
 * the "mpdevs" command on the receiver.  Mpdevs gives output in the
 * following format:
 *
 * <pre>
 * Available 1 local mport(s):
 * +++ mport_id: 0 dest_id: 0
 * RIODP: riomp_mgmt_get_ep_list() has 1 entries
 *         1 Endpoints (dest_ID): 1
 * </pre>
 *
 * In this case, the destination ID of the node is 0.
 *
 * \subsection goodput_cmd_overview_secn Goodput Measurement Display
 *
 * Goodput (amount of data transferred) measurements are displayed by the
 * "goodput" command.
 *
 * <pre>
 *  W STS <<<<--Data-->>>> --MBps-- -Gbps- Messages  Link_Occ
 *  0 Run        2e7800000  198.288  1.586         0   1.670
 *  1 Run        2e7800000  198.321  1.587         0   1.670
 *  2 Run        2e7c00000  198.338  1.587         0   1.670
 *  3 Run        2e7c00000  198.371  1.587         0   1.670
 *  4 Run        2e7800000  198.313  1.587         0   1.670
 *  5 Run        2e7c00000  198.330  1.587         0   1.670
 *  6 Run        2e7c00000  198.346  1.587         0   1.670
 *  7 Run        2e7800000  198.297  1.586         0   1.670
 *  8 ---                0    0.000  0.000         0   0.000
 *  9 ---                0    0.000  0.000         0   0.000
 * 10 ---                0    0.000  0.000         0   0.000
 * 11 ---                0    0.000  0.000         0   0.000
 * Total        173d000000 1586.604 12.693         0  13.361
 * </pre>
 *
 * The columns have the following meanings:
 * - W - Worker thread index, or "Total", which gives totals for all workers
 * - STS - Status of the worker thread (Run, Halt, or Dead (---))
 * - Data - Hexadecimal value for the number of bytes transferred
 * - MBps - Decimal display of the data transfer rate, in megabytes per second
 * - Gbps - Decimal display of the data transfer rate, in gigabits per second
 * - Messages - Count of the number of messages, 0 for DMA and 
 *             Direct I/O measurements
 * - Link_Occ - Decimal display of the RapidIO link occupancy, in gigabits per
 *             second.  Link occupancy includes RapidIO packet header data.
 *
 * \subsection latency_cmd_overview_secn Latency Measurement Display
 *
 * The "lat" command displays latency measurements.  Typically, 
 * latency measurements should be taken with a single worker thread to ensure
 * accuracy.  An example output of the "lat" command is shown below.
 *
 * <pre>
 *  W STS ((((-Count--)))) ((((Min uSec)))) ((((Avg uSec)))) ((((Max uSec))))
 *  0 Run          3567504           11.311           16.717        11673.526
 *  1 ---                0            0.000            0.000            0.000
 *  2 ---                0            0.000            0.000            0.000
 *  3 ---                0            0.000            0.000            0.000
 *  4 ---                0            0.000            0.000            0.000
 *  5 ---                0            0.000            0.000            0.000
 *  6 ---                0            0.000            0.000            0.000
 *  7 ---                0            0.000            0.000            0.000
 *  8 ---                0            0.000            0.000            0.000
 *  9 ---                0            0.000            0.000            0.000
 * 10 ---                0            0.000            0.000            0.000
 * 11 ---                0            0.000            0.000            0.000
 * </pre>
 *
 * The columns have the following meanings:
 * - W - Worker thread index
 * - STS - Status of the worker thread (Run, Halt, or Dead (---))
 * - Count - Decimal display of the number of transactions measured
 * - Min uSec - Decimal display of the smallest latency seen over all 
 * transactions, displayed in microseconds.
 * - Avg uSec - Decimal display of the average latency seen over all 
 * transactions, displayed in microseconds.
 * - Max uSec - Decimal display of the largest latency seen over all 
 * transactions, displayed in microseconds.
 *
 * \subsection script_gen_instr_secn Goodput Script Generation Getting Started
 *
 * This description assumes that a node named IND02 is the target node,
 * and IND01 is the source node for the performance scripts.
 *
 * -# On IND02, run the bash script "goodput/scripts/create_start_scripts.sh" 
 *    as shown below, where
 *    - mport# : is the index of the master port to use on IND02, usually 0
 *    - ### : the first 3 digits of the socket numbers used for 
 *            performance measurement.  Any 3 numbers can be used
 *    - IBA_addr: The hexadecimal RapidIO address used as a 
 *                target for read and write transactions.
 *
 *    ./create_start_scripts {mport#} {socket} {IBA_addr}
 *
 *    For example, './create_start_scripts 0 123 200000000' uses master port
 *    0, socket numbers 123_, and a RapidIO address of 0x200000000.
 *
 *    Create_start_scripts.sh creates two directries:
 *    - goodput/mport{mport#}, which contains the startup scripts 
 *      specific to mport#
 *    - logs/mport{mport#}, used to store any local log files necessary for
 *      correct script execution.
 *
 * -# On IND02, execute goodput with the same {mport} value used
 *  for the create_start_scripts.sh script.
 *
 * -# At the goodput command prompt on IND02, enter:
 *    ". start_target". 
 *
 *    This will display information in the following format:
 *
 * <pre>
 * W STS CPU RUN ACTION MODE IB (((( HANDLE )))) ((((RIO ADDR)))) ((((  SIZE  ))))
 * 0 Run Any Any MSG_Rx KRNL  0                0                0                0
 * 1 Run Any Any MSG_Rx KRNL  0                0                0                0
 * 2 Run Any Any MSG_Rx KRNL  0                0                0                0
 * 3 Run Any Any MSG_Rx KRNL  0                0                0                0
 * 4 Run Any Any MSG_Rx KRNL  0                0                0                0
 * 5 Run Any Any MSG_Rx KRNL  0                0                0                0
 * 6 Run Any Any MSG_Rx KRNL  0                0                0                0
 * 7 Run Any Any MSG_Rx KRNL  0                0                0                0
 * 8 Run Any Any mR_Lat KRNL  0                0                0                0
 * 9 Hlt Any Any NO_ACT KRNL  0                0                0                0
 *10 Hlt Any Any  IBWIN KRNL  1    AAAAAAAAAAAAA       {IBA_addr}           400000
 *11 Run Any Any CpuOcc KRNL  0                0                0                0
 * Available 1 local mport(s):
 * +++ mport_id: 0 dest_id: -->> DID <<--
 * RIODP: riomp_mgmt_get_ep_list() has 1 entries
 *        1 Endpoints (dest_ID): 1
 * script start_target completed, status 0
 * </pre>
 * -# On IND01, run the bash script "goodput/scripts/create_start_scripts.sh" 
 *    as shown, where
 *    - mport : is the index of the master port to use on IND01, usually 0
 *    - ### : the first 3 digits of the socket numbers used for 
 *            performance measurement.  Any 3 numbers can be used.  These can
 *            be different numbers than those used for create_start_scripts.sh
 *            on IND02.
 *    - IBA_addr: The hexadecimal RapidIO address used as a 
 *                target for read and write transactions.
 *
 *    This command is necessary to create the local mport{MPNUM} and logs/mport{MPNUM}
 *    directories used by scripts created in the next step.
 *
 * -# On IND01 run the bash script "scripts/create_perf_scripts.sh" as follows: 
 *  ./create_perf_scripts.sh {WAIT} {DMA_TRANS} {DMA_SYNC} {DID}  {socket} {IBA_addr} {mport#} .
 *  - {DID} is the DID value displayed by IND02
 *  - {IBA_addr} is the same address value used for create_start_scripts.sh on 
 *    IND02
 *  - {socket} is the same 3 digits entered for create_start_scripts.sh on IND02
 *  - For a detailed description of the other parameters, refer to \ref
 *   script_gen_detail_secn 
 *
 * \subsection script_exec_detail_secn Goodput Script Execution
 *
 * All performance measurement scripts are generated by the
 * "create_perf_scripts.sh bash" script.  The scripts are found in the 
 * goodput/mport{MPNUM} directory, along with subdirectories of scripts for
 * each type of measurement.
 *
 * All performance measurement scripts, except DMA write and OBWIN write 
 * latency, are executed by the "scripts/run_all_perf script".
 *
 * For information on measuring DMA write latency, see 
 * \ref dma_lat_scr_secn. 
 * For information on measuring OBWIN write latency, see 
 * \ref dio_lat_scr_secn.
 *
 * The top-level scripts listed below run all of the performance measurement
 * scripts in the associated directory. The top-level scripts are found
 * in the goodput/mport{MPNUM} directory, and are named according to the
 * directory and function of scripts that will be executed.
 *
 * Each top-level script generates a file in the 
 * rapidio_sw/utils/goodput/logs/mport{MPNUM} directory.  The log files are named
 * "topLevel.log", where "topLevel" is the name of the top-level script that
 * generated the log file.  The log file captures the performance measurements
 * for all of the individual scripts run by the top-level script.
 *
 * The create_start_scripts.sh bash script also places four analysis scripts
 * into the logs/mport{MPNUM} directory:
 * - summ_lat_logs.sh : Summarizes the contents of each latency .log file
 *   into a corresponding .res file in the current directory.  Displays the
 *   contents of all latency .res files, so that they can be captured into
 *   another file.
 * - check_lat_logs.sh : Accepts the output of the summ_lat_logs.sh scripts,
 *   and creates two files:
 *   - lat_pass.out : Contains indications of which sizes of transactions 
 *     were successfully measured.
 *   - lat_fail.out : Contains indications of which transactions failed.
 * - summ_thru_logs.sh : Summarizes the contents of each throughput .log file
 *   into a corresponding .res file in the current directory.  Displays the
 *   contents of all throughput .res files, so that they can be captured into
 *   another file.
 * - check_thru_logs.sh : Accepts the output of the summ_thru_logs.sh scripts,
 *   and creates two files:
 *   - thru_pass.out : Contains indications of which sizes of transactions 
 *     were successfully measured.
 *   - thru_fail.out : Contains indications of which transactions failed.
 *
 * Note that the "start_target" script must be running on the target node,
 * and the scripts on the source node must be generated with information 
 * consistent with the target node, to successfully execute the 
 * "run_all_perf" script or any of the top-level scripts on the source.
 *
 * The list of top-level scripts includes the following:
 *
 * - msg_lat_tx  - Messaging latency, requires 
 *                 mport{MPNUM}/msg_lat/m_rx.txt to run on target
 * - msg_thru_tx - Messaging throughput, requires
 *                 mport{MPNUM}/msg_thru/m_rx.txt to run on target
 * - dma_lat_read - Single-thread DMA read latency, 
 *                 requires inbound window allocation on target (run
 *                 mport{MPNUM}/start_target)
 * - dma_thru_read - Single-thread DMA read throughput, 
 *                   requires inbound window allocation on target (run
 *                   mport{MPNUM}/start_target)
 * - dma_thru_write - Single-thread DMA write throughput,
 *                 requires inbound window allocation on target (run
 *                    mport{MPNUM}/start_target)
 * - obwin_thru_write - Outbound window (direct I/O) write throughput
 *                 requires inbound window allocation on target (run
 *                    mport{MPNUM}/start_target)
 * - obwin_thru_read - Outbound window (direct I/O) read throughput, 
 *                     requires inbound window allocation on target (run
 *                    mport{MPNUM}/start_target)
 * - obwin_lat_read - Outbound window (direct I/O) read latency,
 *                     requires inbound window allocation on target (run
 *                    mport{MPNUM}/start_target)
 * - pdma_thru_pd1_read - Multi-threaded DMA read throughput, one DMA engine,
 *                     requires inbound window allocation on target (run
 *                    mport{MPNUM}/start_target)
 * - pdma_thru_pd1_write - Multi-threaded DMA write throughput, one DMA engine
 *                     requires inbound window allocation on target (run
 *                    mport{MPNUM}/start_target)
 * - pdma_thru_pdd_read - Multi-threaded DMA read throughput,
 *                        one DMA engine per thread,
 *                     requires inbound window allocation on target (run
 *                    mport{MPNUM}/start_target)
 * - pdma_thru_pdd_write - Multi-threaded DMA write throughput,
 *                        one DMA engine per thread
 *                     requires inbound window allocation on target (run
 *                    mport{MPNUM}/start_target)
 * - pdma_thru_pdm_read - Multi-threaded DMA read throughput,
 *                        some threads share the same DMA engine,
 *                     Requires inbound window allocation on target (run
 *                    mport{MPNUM}/start_target)
 * - pdma_thru_pdm_write - Multi-threaded DMA write throughput,
 *                        some threads have their own DMA engine
 *                     Requires inbound window allocation on target (run
 *                    mport{MPNUM}/start_target)
 *
 * \subsection script_gen_detail_secn Goodput Script Generation Details
 *
 * The goodput tool set makes use of bash scripts to generate CLI scripts.
 * Bash scripts are generated from a Linux command prompt.  CLI scripts
 * are executed by the goodput tools.
 *
 * The bash script "goodput/scripts/create_perf_scripts.sh" creates
 * a complete set of CLI scripts that can be used to evaluate the performance
 * of a platform. The scripts are specific to a master port device 
 * (/dev/rio_mport{MPNUM}, and so are found in the goodput/mport{MPNUM} directory.
 * Individual CLI scripts are found in one of the mport{MPNUM} subdirectories 
 * listed
 * below. For more information on the other scripts generated, see
 * \ref script_exec_detail_secn.
 *
 * - dma_lat - DMA latency measurement
 * - dma_thru - DMA goodput measurement for a single thread
 * - pdma_thru - Parallel DMA goodput measurement, using multiple threads
 * - msg_lat - Messaging latency measurement
 * - msg_thru - Messaging throughput measurement
 * - obwin_thru - Direct I/O latency measurement with one or multiple threads
 * - obwin_lat - Direct I/O latency measurement
 *
 * Each subdirectory listed above contains a bash script,
 * "create_scripts.sh", and one
 * or more template files used by the bash script as the basis of the
 * scripts to be created.  Each "create_scripts.sh" file accepts parameters
 * that are used to replace keywords in the template file(s).
 *
 * The "create_perf_scripts.sh" script accepts the following parameters:
 * - WAIT - Time in seconds to wait before performance measurement
 * - DMA_TRANS - DMA transaction type
 *                0 = NW, 1 = SW, 2 = NW_R, 3 = SW_R 4 = NW_R_ALL
 * - DMA_SYNC   - 0 = blocking, 1 = async, 2 = fire and forget
 *                NOTE: Fire-and-forget can result in a "burst" of
 *                transactions as the DMA transaction queue is
 *                filled when the measurement begins.  It is recommended to
 *                clear the throughput results with the "goodput clear" CLI
 *                command before taking measurements for fire-and-forget
 *                transactions.
 * - DID        - Device ID of target device for performance scripts
 * - IBA_ADDR   - Hexadecimal address of target window on DID
 * - SKT_PREFIX - First 3 digits of 4-digit socket numbers
 * 
 * - Optional parameters, if not entered same as DMA_SYNC
 *   - MPORT      - Mport number for these scripts, default is 0
 *   - DMA_SYNC2  - 0 = blocking, 1 = async, 2 = fire and forget
 *                  Same as DMA_SYNC.  Used for threads sharing a DMA engine
 *                  for "mixed"  parallel DMA measurements.  For more 
 *                  information, see \ref dma_thruput_scr_secn.
 *   - DMA_SYNC3  - 0 = blocking, 1 = async, 2 = fire and forget
 *                  Same as DMA_SYNC.  Used for threads with a dedicated
 *                  DMA engine
 *                  for "mixed"  parallel DMA measurements.  For more 
 *                  information, see \ref dma_thruput_scr_secn.
 *
 * These parameters are then passed to the "create_scripts.sh" bash scripts
 * in each subdirectory, as appropriate.  
 * The "create_scripts.sh" scripts can also be run on their own. Each script 
 * will describe the parameters it accepts if called without any parameters.
 * 
 * \subsection dio_meas_secn Direct I/O Measurement
 * Direct I/O read and write transactions are generally performed 
 * as processor reads and writes, so the 
 * transaction sizes are restricted to 1, 2, 4, and 8 bytes.  Any other 
 * transaction size can be used to measure the overhead of the goodput 
 * infrastructure for measuring goodput/latency.
 * 
 * Direct I/O measurements require two threads: one on the source of the
 * transactions, and one on the target. 
 *
 * The target thread must have allocated an inbound window using the "IBAlloc"
 * command.  
 * The source thread must be commanded to send data to the inbound window
 * address of the target, and to use the destination id of the target;
 *
 * The inbound window address of the target is displayed by the "status" command. 
 * For more information, see \ref stat_secn.
 * The destination ID of the target is displayed by the mpdevs command.  
 * For more information, see \ref destID_overview_secn.
 *
 * \subsubsection dio_thruput_scr_secn Direct I/O Goodput Measurement Scripts
 *
 * All direct I/O read measurements can be performed by executing the
 * "mport{MPNUM}/obwin_thru_read" top-level script from the Goodput
 * command prompt.
 * The performance results are captured in the "obwin_thru_read.log" file,
 * found in the rapidio_sw/utils/goodput/logs/mport{MPNUM} directory.
 *
 * All direct I/O write measurements can be performed by executing the
 * "mport{MPNUM}/obwin_thru_write" top-level script from the Goodput
 * command prompt.
 * The performance results are captured in the "obwin_thru_write.log" file,
 * found in the rapidio_sw/utils/goodput/logs/mport{MPNUM} directory.
 *
 * Direct I/O goodput measurement scripts are found in the
 * mport{0}/obwin_thru directory.  The script name format is
 * oNdSZ.txt, where:
 *
 * - N is the number of threads performaning the access, either 1 or 8
 * - d is the direction, either R for read or W for write
 * - SZ is the size of the access, consisting of a number of bytes followed by
 *   one of B for bytes, K for kilobytes, or M for megabytes.
 *
 * For example, script "goodput/mport{MPNUM}/obwin_thru/o1W4B.txt"
 * executes a single thread performing 4 byte writes.
 *
 * \subsubsection dio_thruput_secn Direct I/O Goodput Measurement
 *
 * The "OBDIO" command measures goodput for Direct I/O transactions.
 * For example, to measure the goodput for 8 byte-write accesses to an
 * inbound window 0x400000 bytes in size, located at address 0x22f000000 found
 * on the device with destination ID of 9, using thead 3, type:
 *
 * OBDIO 3 9 22f000000 400000 8 1
 *
 * For more examples, see the generated direct I/O measurement scripts.
 *
 * \subsubsection dio_lat_scr_secn Direct I/O Latency Measurement Scripts
 *
 * Scripts to perform direct I/O latency measurement are found in the
 * "mport{0}/obwin_lat" directory.  All script names have the 
 * format olDsz, where:
 *
 * - D is the direction, either R for read, W for write, or T for looping
 *   back received write data.
 * - sz is the size of the access, consisting of a number of bytes followed by
 *   one of B for bytes, K for kilobytes, or M for megabytes.
 *
 * For example, script "goodput/mport{MPNUM}/obwin_lat/olT2B.txt" is
 * executed on the target to loop back 2 byte writes.
 *
 * To execute all of the read latency scripts, complete the following steps:
 * 1. Execute the script "mport{MPNUM}/start_target" on the target.
 * 2. Execute the script "mport{MPNUM}/obwin_lat_read" command on the
 * source.
 * 
 * All direct I/O read latency measurements are captured in the
 * "rapidio_sw/utils/goodput/logs/mport{MPNUM}/obdio_read_lat.log" file.
 *
 * To execute individual write latency measurements, complete the following
 * steps:
 * -# Kill all workers on the source node with the "kill all" command.
 * -# Execute the "mport{MPNUM}/obwin_lat/olTnB.txt" script on the target.
 * -# Execute the "mport{MPNUM}/obwin_lat/olWnB.txt" script on the source
 *    node that will measure latency.
 *
 * The latency measurement result will be shown in the source node CLI session.
 * All direct I/O write latency measurements cannot be executed 
 * from a top-level CLI script.  
 *
 * \subsubsection dio_lat_secn Direct I/O Latency Measurement CLI Commands
 *
 * Latency measurement for direct I/O read transactions can be performed by
 * the source node without assistance from the target node.  For example, to
 * measure the latency of 2 byte-read accesses to an inbound window 0x200000
 * bytes in size, located at address 0x22f000000 found on the device with
 * destination ID of 9, using thread 4, type:
 *
 * DIOTxLat 4 9 22f000000 2 0
 *
 * Direct I/O write latency measurements transactions require a thread on
 * the target node to "loop back" the write performed by the source node.
 *
 * To measure write latency, complete the following steps:
 * -# Start a thread on the source node that will be used to measure
 *    write latency.
 * -# Allocate an inbound window using the "IBAlloc" command for the thread
 *    on the source node.
 * -# Start a thread on the target node that will be used to loop back
 *    received write data.
 * -# Allocate an inbound window using the "IBAlloc" command for the thread
 *    on the source node.
 * -# Execute the DIORxLat command on the target node.
 * -# Execute the DIOTxLat command on the source node. 
 *
 * For example, on the target, initiate the "loop back" for 4 byte writes from
 * the source with destination ID of 7, being written to the sources inbound
 * window at address 0x22f400000, using thread 6:
 *
 * DIORxLat 6 7 22f400000 4
 *
 * On the source, initiate the 4 byte writes to the receiver with destination
 * ID of 8 using thread 3:
 *
 * DIOTxLat 3 8 22f400000 4 1
 *
 * \subsection dma_meas_secn DMA Measurements
 *
 * DMA read and write goodput measurements are performed as a sequence of
 * smaller transactions that add up to a total number of bytes.  Once the
 * total number of bytes specified has been transferred, the DMA goodput
 * statistics are updated.  For example, 4 MB of data can be transferred
 * as a single DMA transaction, or as a sequence of 64 KB transactions.
 * The sequence of 64 KB transactions would require more DMA descriptors and
 * more processing, therefore the goodput for smaller transactions is generally 
 * lower than for larger transactions.
 *
 * Similarly, DMA transactions can be performed using kernel buffers
 * (contiguous physical memory) or user mode buffers (discontiguous memory).
 * User mode buffers generally give lower goodput, since they are
 * discontiguous physical memory and so require more DMA
 * descriptors/transactions to effect a transfer.
 * 
 * DMA measurements require two threads: one on the source of the
 * transactions, and one on the target.
 *
 * The target must have allocated an inbound window using the "IBAlloc"
 * command.  
 * The target thread must have allocated an inbound window using the "IBAlloc"
 * command.  
 *
 * The source thread must be commanded to send data to the inbound window
 * address of the target, and to use the destination id of the target.
 *
 * The inbound window address of the target is displayed by the "status" command. 
 * For more information, see \ref stat_secn.
 * The destination ID of the target is displayed by the "mpdevs" command.  
 * For more information, see \ref destID_overview_secn.
 *
 * \subsubsection dma_thruput_scr_secn DMA Goodput Measurement Scripts
 *
 * DMA Goodput measurement scripts are found in two directories:
 * dma_thru and pdma_thru.  Scripts in dma_thru use a single thread to
 * send packets to a single DMA queue.  Scripts in pdma_thru use 8 threads
 * to send packets from multiple DMA queues.  Depending on the capabilities of
 * the processor and the endpoint, pdma_thru scripts may provide more
 * goodput than dma_thru.
 *
 * Pdma_thru measurements vary based on the number of DMA engines used. 
 * The number of DMA engines used is captured in the name of the pdma_thru
 * top-level script:
 *
 * pdma_thru_pd1_read - Single DMA engine, multiple threads, read
 * pdma_thru_pd1_write - Single DMA engine, multiple threads, write
 * pdma_thru_pdd_read - Multiple DMA engines, single thread per engine, read
 * pdma_thru_pdd_write - Multiple DMA engines, single thread per engine, write
 * pdma_thru_pdm_read - Mix of single and multiple threads per engine, read
 * 4 threads share a DMA engine, and 4 threads each havir their own DMA
 * engine.
 * pdma_thru_pdm_write - Mix of single and multiple threads per engine, write
 * 4 threads share a DMA engine, and 4 threads each havir their own DMA
 * engine.
 *
 * All DMA read measurements can be performed by executing the
 * "mport{0}/dma_thru_read" or "pdma_thru_XXX_read" scripts 
 * from the Goodput command prompt. The performance results are
 * captured in the "dma_thru_read.log" and "pdma_thru_XXX_read.log" files,
 * found in the rapidio_sw/utils/goodput/logs/mport{MPNUM} directory.
 *
 * All DMA write measurements can be performed by executing the
 * "mport{0}/dma_thru_write" or "pdma_thru_XXX_write" scripts 
 * from the Goodput command prompt. The performance results are
 * captured in the dma_thru_write.log and dma_thru_XXX_write.log files,
 * found in the rapidio_sw/utils/goodput/logs/mport{MPNUM} directory.
 *
 * DMA goodput measurement scripts are found in the
 * mport{MPNUM}/dma_thru and pdma_thru directories.
 * The script name format is pfxTsz.txt, where:
 *
 * - pfx indicates the kind of parallelism executing:
 *   - pd1 - Single DMA engine, multiple threads
 *   - pdd - Multiple DMA engines, single thread per engine
 *   - pdm - Mix of single and multiple threads per engine
 *   - d1 - Single DMA engine, single thread
 * - T is the type of access, either R for read or W for write
 * - sz is the size of the access, consisting of a number of followed by
 *   one of B for bytes, K for kilobytes, or M for megabytes.
 *
 * For example, the file "goodput/mport{MPNUM}/pdma_thru/pdmR256K.txt"
 * indicates that this script measures dma goodput for a
 * mix of single and multiple threads per engine using 256 kilobyte transfers.
 *
 * \subsubsection dma_thruput_secn DMA Goodput Measurement CLI Command
 *
 * The "dma" command measures goodput for Direct I/O transactions.
 * For example, to measure the goodput for 4 MB write accesses to an
 * inbound window 0x400000 (4 MB) bytes in size located at address
 * 0x22f000000 found
 * on the device with destination ID of 9, using thead 3, type:
 *
 * dma 3 9 22f000000 400000 400000 1 1 0 0
 *
 * \subsubsection dma_lat_scr_secn DMA Latency Measurement Scripts
 *
 * Scripts to perform DMA latency measurement are located in the
 * "mport{MPNUM}/dma_lat" directory.  All script names have the 
 * format dlDsz, where:
 *
 * - D is the direction, either R for read, W for write, or T for looping
 *   back received write data.
 * - sz is the size of the access, consisting of a number followed by
 *   one of B for bytes, K for kilobytes, or M for megabytes.  For example,
 *   file dlT2M.txt is the script to run on the target for 2 megabyte DMA
 *   transfers.
 *
 * To execute all of the read latency scripts, complete the following:
 * -# Execute the script "mport{MPNUM}/start_target" on the target.
 * -# Execute the script "mport{MPNUM}/dma_lat_read" on the source.
 * 
 * All DMA read latency measurements are captured in the
 * "rapidio_sw/utils/goodput/logs/mport{MPNUM}dma_read_lat.log" file.
 *
 * To execute individual write latency measurements, complete the following
 * steps:
 * 1. Kill all workers on the source node with the "kill all" command.
 * 2. Execute the goodput/mport{MPNUM}/obwin_lat/dlTnB.txt script on the target.
 * 3. Execute the goodput/mport{MPNUM}/obwin_lat/dlWnB.txt script on the source
 * node that will measure latency.
 *
 * The latency measurement result will be shown in the source node CLI session.
 *
 * All DMA write latency measurements cannot be executed 
 * from a single CLI script.  
 *
 * \subsubsection dma_lat_secn DMA Latency Measurement CLI Commands
 *
 * Latency measurement for DMA read transactions can be performed by
 * the source node without assistance from the target node.  For example, to
 * measure the latency of 0x10 byte read accesses to an inbound window 0x200000
 * bytes in size located at address 0x22f000000 found on the device with
 * destination ID of 9, using thread 4, use the dTxLat command as follows:
 *
 * dTxLat 4 9 22f000000 10 0 1 0
 *
 * Latency measurement for DMA write transactions requires a thread on
 * the target node to "loop back" the write performed by the source node.
 * Measuring write latency is a two-stage process: Start the "loop back"
 * thread command running on the receiver using the dRxLat command, then 
 * initiate transmission on the source thread with the dTxLat command.
 *
 * On the receiver, initiate the "loop back" for 0x200000 byte writes from the 
 * source with destination ID of 7, being written to the receiver's inbound
 * window at address 0x22f400000, using thread 6:
 *
 * dRxLat 6 7 22f400000 200000 
 *
 * On the source, initiate the 0x200000 byte writes to the receiver with
 * destination
 * ID of 8 using thread 3:
 *
 * dTxLat 3 8 22f400000 200000 1 1 0 
 *
 * \subsection msg_meas_secn Messaging Measurement
 *
 * Unlike Direct I/O and DMA measurements, messaging measurements always
 * require a running receiving thread to perform any measurements.
 *
 * The receiving thread for goodput is run using the "msgRx" command,
 * and the source  thread is run using the "msgTx" command.
 * The receiving thread for latency is run using the "mRxLat" command,
 * and the source thread is run using the "mTxLat" command.
 * 
 * Note that the minimum message size is 24 bytes, and the maximum is 4096
 * bytes. A message must always be a multiple of 8 bytes in size.
 * 
 * The source thread must be commanded to use the destination ID of the target.
 * The destination ID of the target is displayed by the "mpdevs" command.  
 * For more information, see \ref destID_overview_secn.
 *
 * \subsubsection msg_thruput_scr_secn Messaging Goodput Measurement Scripts
 *
 * Messaging goodput measurement scripts are found in the
 * "mport{MPNUM}/msg_thru" directory.  To execute all messaging
 * performance scripts, complete the following steps:
 *
 * -# Execute the "mport{MPNUM}/start_target" script on the target node.
 * -# Execute the "mport{MPNUM}/msg_thru_tx.txt" script on the
 *    source node.
 *
 * All messaging throughput measurements are captured in the
 * "rapidio_sw/utils/goodput/logs/mport{MPNUM}/msg_thru_tx.log" file.
 *
 * \subsubsection msg_thruput_secn Messaging Goodput Measurement CLI Commands
 *
 * First, use receive thread 3 on the node with destID 5 and socket number 1234
 * as follows:
 *
 * msgRx 3 1234
 *
 * On the transmitting node, use the "msgTx" command to send 4096 byte messages 
 * using thread 7 to the node with the msgRx thread as follows:
 *
 * msgTx 7 5 1234 4096
 *
 * \subsubsection msg_latency_scr_secn Messaging Latency Measurement Scripts
 *
 * Messaging latency measurement scripts are found in the
 * "mport{MPNUM}/msg_lat" directory.  All script names have the 
 * format mTsz, where:
 *
 * - sz is the size of the access, consisting of a number followed by
 *   one of B for bytes, K for kilobytes, or M for megabytes.  
 *   Message sizes must be a minimum of 24 bytes, and a maximum of 4096 bytes
 *   (4K).
 *
 * To execute all messaging latency scripts, complete the following stesp:
 *
 * -# Execute the "mport{MPNUM}/start_target" script on the target node.
 * -# Execute the "mport{MPNUM}/msg_lat_tx.txt" script on the
 *    source node.
 *
 * \subsubsection msg_latency_secn Messaging Latency Measurement CLI Commands
 *
 * First, use receive thread 3 on the node with destID 5 and socket number 1234
 * to send back 2048 byte messages to the source.  The "mRxLat" command looks 
 * like the following:
 *
 * mRxLat 3 1234 2048
 *
 * On the transmitting node, use the "mTxLat" command to send 2048 byte messages 
 * using thread 7 to the node with the mRxLat thread as follows:
 *
 * msgTx 7 5 1234 2048
 */

#ifndef __GOODPUT_INTRO_H__
#define __GOODPUT_INTRO_H__

#endif /* __GOODPUT_INTRO_H__ */
