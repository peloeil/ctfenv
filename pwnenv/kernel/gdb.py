import gdb

gdb.execute("file vmlinux")
gdb.execute("target remote:1234")
#gdb.execute("ksymaddr-remote-apply")
#gdb.execute("kmod-load vuln ./rootdir/root/vuln.ko")
#gdb.execute("ctx")

