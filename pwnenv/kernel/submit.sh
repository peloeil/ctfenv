#!/bin/sh

CFLAGS="-Os -flto -fno-plt -fdata-sections -ffunction-sections -fno-stack-protector -fomit-frame-pointer -fmerge-all-constants -Wl,--gc-sections" ./pack.sh

strip -s ./rootdir/exploit
upx ./rootdir/exploit

./upload.py
