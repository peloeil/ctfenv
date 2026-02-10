import gdb


def find_kbase():
    kbase = int(gdb.execute("kbase", to_string=True).split("\n")[1][15 : 15 + 18], 16)
    phys_base = int(gdb.execute(f"v2p {hex(kbase)}", to_string=True).split()[4], 16)
    return kbase, phys_base


def easy_checksec():
    kbase, phys_base = find_kbase()
    kversion = gdb.execute("kversion", to_string=True).split("\n")[1].split()[1:4]
    kchecksec = gdb.execute("kchecksec", to_string=True).split("\n")

    # mitigations that can be switched in cmdline
    smep_enabled = False
    smap_enabled = False
    cet_enabled = False
    kpti_enabled = False
    kaslr_enabled = False

    # config
    uffd_can_be_used = False
    modprobe_can_be_used = False
    fuse_can_be_used = False

    for i, line in enumerate(kchecksec):
        match i:
            case 8:
                if "Enabled" in line:
                    smep_enabled = True
            case 9:
                if "Enabled" in line:
                    smap_enabled = True
            case 10:
                if "Enabled" in line:
                    cet_enabled = True
            case 12:
                if "Enabled" in line:
                    kaslr_enabled = True
            case 15:
                if "Enabled" in line:
                    kpti_enabled = True
            case 35:
                if "Enabled" in line:
                    uffd_can_be_used = True
            case 54:
                if "Disabled" in line:
                    modprobe_can_be_used = True
            case 57:
                if "Enabled" in line:
                    fuse_can_be_used = True

    print(*kversion)
    print(f"kbase:    {hex(kbase)} (phys: {hex(phys_base)})")
    print(f"SMEP:     {smep_enabled}")
    print(f"SMAP:     {smap_enabled}")
    print(f"CET:      {cet_enabled}")
    print(f"KPTI:     {kpti_enabled}")
    print(f"KASLR:    {kaslr_enabled}")
    print()
    print(f"uffd:     {uffd_can_be_used}")
    print(f"modprobe: {modprobe_can_be_used}")
    print(f"fuse:     {fuse_can_be_used}")
    return


gdb.execute("file ./vmlinux")
gdb.execute("target remote:1234")
gdb.execute("ksymaddr-remote-apply")
gdb.execute("ctx")
# gdb.execute("kmod-load vuln ./rootdir/root/vuln.ko")
easy_checksec()
