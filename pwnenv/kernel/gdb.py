import gdb

gdb.execute("file vmlinux")
gdb.execute("target remote:1234")
