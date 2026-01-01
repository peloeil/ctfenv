#!/bin/sh

# musl static link

# dev cpio
cd rootdir
find . -print0 | cpio -o --format=newc --null --owner=root > ../rootfs_dev.cpio
cd ..
