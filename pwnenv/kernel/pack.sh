#!/bin/sh

# musl static link
musl-gcc -static ${CFLAGS:-} src/exploit.c -o rootdir/exploit
#gcc -S src/exploit.c -o asm.S
#musl-gcc -static ${CFLAGS:-} asm.S -o rootdir/exploit
#rm asm.S

# dev cpio
cd rootdir
find . -print0 | cpio -o --format=newc --null --owner=root > ../rootfs_dev.cpio
cd ..
