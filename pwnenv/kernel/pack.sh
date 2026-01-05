#!/bin/sh

# musl static link
musl-gcc -static ${CFLAGS:-} -o rootdir/exploit src/exploit.c

# dev cpio
cd rootdir
find . -print0 | cpio -o --format=newc --null --owner=root > ../rootfs_dev.cpio
cd ..
