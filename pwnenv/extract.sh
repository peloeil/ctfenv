#!/bin/sh
mkdir debugfs
cd debugfs; cpio -idv < ../rootfs.cpio
cd ../
