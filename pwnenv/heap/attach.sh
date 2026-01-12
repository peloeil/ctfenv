#!/bin/sh

# change pgrep name if you want to attach remotely
PID=$(pgrep <binary-name> | tail -n 1)
echo "$PID"
sudo MY_PID=$PID gdb -x gdb.py
