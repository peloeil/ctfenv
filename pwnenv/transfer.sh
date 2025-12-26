#!/bin/sh
gcc exploit.c -o exploit -static
mv exploit debugfs
cd debugfs; find . -print0 | cpio -o --format=newc --null --owner=root > ../debugfs.cpio
cd ../

qemu-system-x86_64 \
    -kernel bzImage \
    -initrd debugfs.cpio \
    -m 64M \
    -nographic \
    -append "console=ttyS0 loglevel=3 oops=panic panic=-1 nopti nokaslr" \
    -no-reboot \
    -cpu qemu64 \
    -smp 1 \
    -monitor /dev/null \
    -net nic,model=virtio \
    -net user \
    -s
