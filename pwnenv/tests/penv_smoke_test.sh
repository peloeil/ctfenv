#!/usr/bin/env bash

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PENV="$REPO_ROOT/penv"

die() {
    printf 'test: %s\n' "$*" >&2
    exit 1
}

assert_contains() {
    local path="$1"
    local pattern="$2"

    grep -q -- "$pattern" "$path" || die "expected '$pattern' in $path"
}

assert_not_contains() {
    local path="$1"
    local pattern="$2"

    if grep -q -- "$pattern" "$path"; then
        die "did not expect '$pattern' in $path"
    fi
}

assert_exists() {
    local path="$1"

    [ -e "$path" ] || die "expected $path to exist"
}

assert_executable() {
    local path="$1"

    [ -x "$path" ] || die "expected $path to be executable"
}

assert_glob_exists() {
    local pattern="$1"
    local path

    for path in $pattern; do
        [ -e "$path" ] || continue
        return 0
    done
    die "expected a path matching $pattern"
}

assert_missing() {
    local path="$1"

    [ ! -e "$path" ] || die "expected $path to be absent"
}

find_libc() {
    local libc

    libc="$(
        ldd /bin/true 2>/dev/null | awk '/libc\.so\.6/ { print $3; exit }'
    )"
    if [ -n "${libc:-}" ] && [ -f "$libc" ]; then
        printf '%s\n' "$libc"
        return 0
    fi

    libc="$(find /lib /usr/lib /usr/lib64 -name 'libc.so.6' 2>/dev/null | head -n1 || true)"
    [ -n "$libc" ] || die "libc.so.6 not found for tests"
    printf '%s\n' "$libc"
}

make_stub_pwninit() {
    local bin_dir="$1"
    local with_ld="${2:-0}"
    local noisy_stdout="${3:-0}"

    mkdir -p "$bin_dir"
    cat >"$bin_dir/pwninit" <<'EOF'
#!/usr/bin/env bash

set -euo pipefail

bin=""
while [ "$#" -gt 0 ]; do
    case "$1" in
    --bin)
        bin="$2"
        shift 2
        ;;
    --libc)
        shift 2
        ;;
    *)
        shift
        ;;
    esac
done

cp -- "${bin#./}" "${bin#./}_patched"
if [ "${PENV_TEST_STDOUT_WARNING:-0}" = "1" ]; then
    printf 'warning: simulated stdout noise\n'
fi
if [ "${PENV_TEST_WITH_LD:-0}" = "1" ]; then
    : > ld-linux-x86-64.so.2
fi
: > solve.py
EOF
    chmod +x "$bin_dir/pwninit"

    if [ "$with_ld" = "1" ]; then
        export PENV_TEST_WITH_LD=1
    else
        export PENV_TEST_WITH_LD=0
    fi
    if [ "$noisy_stdout" = "1" ]; then
        export PENV_TEST_STDOUT_WARNING=1
    else
        export PENV_TEST_STDOUT_WARNING=0
    fi
}

make_stub_docker_for_jail_libc() {
    local bin_dir="$1"

    mkdir -p "$bin_dir"
    cat >"$bin_dir/docker" <<'EOF'
#!/usr/bin/env bash

set -euo pipefail

if [ "$#" -lt 1 ] || [ "$1" != "compose" ]; then
    printf 'unexpected docker invocation\n' >&2
    exit 1
fi

script="${*: -1}"
case "$script" in
*"/srv/lib/x86_64-linux-gnu/libc.so.6"*"/lib/x86_64-linux-gnu/libc.so.6"*)
    printf 'challenge-libc\n'
    ;;
*)
    printf 'unexpected libc lookup order\n' >&2
    exit 1
    ;;
esac
EOF
    chmod +x "$bin_dir/docker"
}

run_bare_test() {
    local tmpdir

    tmpdir="$(mktemp -d)"
    trap 'rm -rf "$tmpdir"' RETURN

    cp /bin/true "$tmpdir/chall"

    (
        cd "$tmpdir"
        "$PENV" init bare
        assert_contains exploit.py 'ELF("./chall")'
        assert_not_contains exploit.py 'pwntools'
        assert_executable exploit.py
        assert_exists Makefile
        assert_contains compose.yaml '^services:'
        assert_contains compose.yaml '^  pwnenv:'
        assert_contains compose.yaml 'dockerfile: bare.Dockerfile'
        assert_not_contains compose.yaml '/dev/kvm'
        assert_contains bare.Dockerfile 'WORKDIR /home/pwn/bare'
    )
}

run_stack_test() {
    local tmpdir libc

    tmpdir="$(mktemp -d)"
    trap 'rm -rf "$tmpdir"' RETURN

    libc="$(find_libc)"
    make_stub_pwninit "$tmpdir/bin" 0 1
    cp /bin/true "$tmpdir/chall"
    cp "$libc" "$tmpdir/libc-2.39.so"
    cat >"$tmpdir/tmux.conf" <<'EOF'
set -g status off
EOF
    cat >"$tmpdir/compose.yaml" <<'EOF'
services:
  chall:
    image: example/chall
    ports:
      - "31337:1337"
volumes:
  data:
EOF

    (
        cd "$tmpdir"
        PATH="$tmpdir/bin:$PATH" "$PENV" init stack
        assert_contains exploit.py 'chall_patched'
        assert_not_contains exploit.py 'pwntools'
        assert_executable exploit.py
        assert_contains compose.yaml '^  pwnenv:'
        assert_contains compose.yaml '^volumes:'
        assert_contains stack.Dockerfile 'WORKDIR /home/pwn/stack'
        assert_contains tmux.conf 'status off'

        PATH="$tmpdir/bin:$PATH" "$PENV" clean
        assert_glob_exists 'exploit.py.bak.*'
        assert_glob_exists 'Makefile.bak.*'
        if grep -q '^  pwnenv:' compose.yaml; then
            die "pwnenv service should be removed by clean"
        fi
        assert_contains compose.yaml '^volumes:'
        assert_exists libc-2.39.so
        assert_missing libc.so.6
        assert_exists tmux.conf
        assert_missing .penv-state
    )
}

run_stack_extracts_srv_libc_test() {
    local tmpdir

    tmpdir="$(mktemp -d)"
    trap 'rm -rf "$tmpdir"' RETURN

    make_stub_pwninit "$tmpdir/bin" 0
    make_stub_docker_for_jail_libc "$tmpdir/bin"
    cp /bin/true "$tmpdir/chall"
    cat >"$tmpdir/compose.yaml" <<'EOF'
services:
  chall:
    image: example/chall
    ports:
      - "31337:1337"
EOF

    (
        cd "$tmpdir"
        PATH="$tmpdir/bin:$PATH" "$PENV" init stack
        assert_contains libc.so.6 'challenge-libc'
        assert_executable exploit.py
    )
}

run_heap_test() {
    local tmpdir libc

    tmpdir="$(mktemp -d)"
    trap 'rm -rf "$tmpdir"' RETURN

    libc="$(find_libc)"
    make_stub_pwninit "$tmpdir/bin" 1
    cp /bin/true "$tmpdir/chall"
    cp "$libc" "$tmpdir/libc-2.39.so"
    cat >"$tmpdir/Dockerfile" <<'EOF'
ARG VERSION=22.04
FROM ubuntu:${VERSION}
EOF
    cat >"$tmpdir/compose.yaml" <<'EOF'
services:
  chall:
    image: example/chall
    ports:
      - "4444:4444"
networks:
  default: {}
EOF

    (
        cd "$tmpdir"
        PATH="$tmpdir/bin:$PATH" "$PENV" init heap
        assert_contains compose.yaml 'VERSION: "22.04"'
        assert_contains attach.sh 'chall_patched'
        assert_contains exploit.py 'Socket("localhost", 4444)'
        assert_executable exploit.py
        assert_contains compose.yaml 'network_mode: "service:chall"'
        assert_not_contains compose.yaml '/dev/kvm'
        assert_contains compose.yaml '^networks:'
    )
}

run_heap_requires_dockerfile_test() {
    local tmpdir libc

    tmpdir="$(mktemp -d)"
    trap 'rm -rf "$tmpdir"' RETURN

    libc="$(find_libc)"
    make_stub_pwninit "$tmpdir/bin" 1
    cp /bin/true "$tmpdir/chall"
    cp "$libc" "$tmpdir/libc-2.39.so"
    cat >"$tmpdir/compose.yaml" <<'EOF'
services:
  chall:
    image: example/chall
    ports:
      - "4444:4444"
EOF

    (
        cd "$tmpdir"
        if PATH="$tmpdir/bin:$PATH" "$PENV" init heap >/dev/null 2>&1; then
            die "heap init should fail without Dockerfile"
        fi
    )
}

run_info_check_no_side_effect_test() {
    local tmpdir info_output check_output

    tmpdir="$(mktemp -d)"
    trap 'rm -rf "$tmpdir"' RETURN

    cp /bin/true "$tmpdir/chall"
    cat >"$tmpdir/compose.yml" <<'EOF'
services:
  chall:
    image: example/chall
    ports:
      - "31337:1337"
EOF

    (
        cd "$tmpdir"
        info_output="$("$PENV" info)"
        check_output="$("$PENV" check)"
        [ -f compose.yml ] || die "info/check should not rename compose.yml"
        [ ! -f compose.yaml ] || die "info/check should not create compose.yaml"
        printf '%s\n' "$info_output" | grep -q 'compose: compose.yml' || die "info should report compose.yml"
        printf '%s\n' "$check_output" | grep -q 'compose: compose.yml' || die "check should report compose.yml"
    )
}

run_kernel_test() {
    local tmpdir

    tmpdir="$(mktemp -d)"
    trap 'rm -rf "$tmpdir"' RETURN

    mkdir -p "$tmpdir/rootfs-src"
    cat >"$tmpdir/rootfs-src/hello.txt" <<'EOF'
hello
EOF
    (
        cd "$tmpdir/rootfs-src"
        find . | cpio --create --format=newc >"$tmpdir/initramfs.cpio"
    )
    cat >"$tmpdir/run.sh" <<'EOF'
#!/bin/sh
qemu-system-x86_64 -initrd initramfs.cpio
EOF
    chmod +x "$tmpdir/run.sh"

    (
        cd "$tmpdir"
        "$PENV" init kernel
        assert_exists initramfs.cpio
        assert_missing rootfs.cpio
        assert_exists rootdir/hello.txt
        assert_contains compose.yaml '/dev/kvm'
        : > rootdir/stale.txt

        "$PENV" init kernel
        assert_missing rootdir/stale.txt
        assert_exists rootdir/hello.txt
    )
}

run_bare_test
run_stack_test
run_stack_extracts_srv_libc_test
run_heap_test
run_heap_requires_dockerfile_test
run_info_check_no_side_effect_test
run_kernel_test

printf 'penv smoke tests passed\n'
