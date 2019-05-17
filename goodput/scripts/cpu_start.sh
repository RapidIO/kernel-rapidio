#!/bin/bash
echo 1 | sudo tee /sys/devices/system/cpu/cpu1/online
echo 1 | sudo tee /sys/devices/system/cpu/cpu2/online
echo 1 | sudo tee /sys/devices/system/cpu/cpu3/online
lscpu
