import gdb
import os

pid = os.environ.get("MY_PID")

gdb.execute("set sysroot")
gdb.execute(f"attach {pid}")
