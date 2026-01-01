#!/bin/sh

PID=$(pgrep <binary-name> | tail -n 1)
echo "$PID"
sudo MY_PID=$PID gdb -x gdb.py
