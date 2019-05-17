#!/bin/bash
cp ../../../common/libmport/riodp_test_misc .
./riodp_test_misc -o 0x148 -w -V 0
./riodp_test_misc -o 0x10080 -w -V 0x10000000
echo "Loopback       0x10000000 expected"
./riodp_test_misc -o 0x10080
echo "Port status    0x00000002 expected"
./riodp_test_misc -o 0x158


