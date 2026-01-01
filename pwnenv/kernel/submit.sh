#!/bin/sh

./pack.sh

strip -s ./rootdir/exploit
upx ./rootdir/exploit

./upload.py
