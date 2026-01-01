# pwnenv
CTF の pwn 用の個人的な docker 環境

## 前提
`penv` を起動するディレクトリでは以下のファイルが存在することを前提とする。

- stack 問題の場合
    - 問題バイナリ
    - (`Dockerfile` and compose file) or libc
- heap 問題の場合
    - 問題バイナリ
    - `Dockerfile`
    - compose file
    - libc (optional)
- kernel 問題の場合
    - `bzImage`
    - startup script
    - cpio file
    - `vmlinux` (optional)
    - `Dockerfile` (optional)
    - `compose.yaml` (optional)

## 使い方
まず `penv` にパスを通す。

- `penv init (bare|stack|heap|kernel)`
    その問題における環境構築を行う。
    - bare
        - `exploit.py` を用意する
        - exploit 用コンテナの `compose.yaml` を用意する
            - 問題バイナリ、`exploit.py` はすべて `/home/pwn` にマウントされる
    - stack
        - libc がない場合は問題用コンテナから libc を抽出する
        - pwninit で libc を unstrip する
        - `exploit.py` を用意する
        - デバッグシンボル付き libc のファイル名を`libc.so.6` に統一する
        - compose file があれば、そのファイル名を `compose.yaml` に統一する
        - `compose.yaml` があれば、`compose.yaml` に問題用コンテナと exploit 用コンテナを一緒に起動する設定を追加する
            - exploit 用コンテナでは `nc <problem-service-name> <port-number>` で問題用コンテナに接続できる
            - 問題バイナリ, `libc.so.6`, `exploit.py` はすべて `/home/pwn` にマウントされる
    - heap
        - libc がない場合は問題用コンテナから libc を抽出する
        - pwninit で libc を unstrip し、対応する ld も用意する
        - `gdb.py`, `attach.sh`, `exploit.py` を用意する
        - デバッグシンボル付き libc のファイル名を`libc.so.6` に統一する
        - ld の名前を `ld-linux-x86-64.so.2` に統一する
        - compose file の名前を `compose.yaml` に統一する
        - `compose.yaml` に問題用コンテナと exploit 用コンテナを一緒に起動する設定を追加する
            - exploit 用コンテナでは `nc <problem-service-name> <port-number>` で問題用コンテナに接続できる
            - 問題バイナリ, `libc.so.6`, `ld-linux-x86-64.so.2`, `gdb.py`, `attach.sh`, `exploit.py` はすべて `/home/pwn` にマウントされる
    - kernel
        - `vmlinux` がない場合は `extract-vmlinux` で `vmlinux` を抽出する
        - cpio を `rootdir` ディレクトリに展開する
        - `rootdir/exploit.c`, `gdb.py`, `dev.sh`, `pack.sh`, `submit.sh`, `upload.py` を用意する
        - startup script のファイル名を `run.sh` に統一する
        - cpio のファイル名を `rootfs.cpio` に統一する
            - `run.sh` 内のファイル名も変更する
        - compose file があれば、そのファイル名を `compose.yaml` に統一する
        - `compose.yaml` があれば、`compose.yaml` に問題用コンテナと exploit 用コンテナを一緒に起動する設定を追加する
            - exploit 用コンテナでは `nc <problem-service-name> <port-number>` で問題用コンテナに接続できる
            - `bzImage`, `vmlinux`, `run.sh`, `dev.sh`, `pack.sh`, `submit.sh`, `upload.py`, `gdb.py`, `rootdir/`, `rootfs.cpio`, `rootfs_dev.cpio` はすべて `/home/pwn` にマウントされる
- `penv up`
    - 問題用コンテナと exploit 用コンテナを開始する (port は調整する必要あり)
- `penv run`
    - exploit 用コンテナに入る

## 詳細
- tmux + neovim on fish で編集
    - 設定は自分の dotfiles から取ってきている
- gdb
    - bata24/gef
- exploitation framework
    - pwntools
    - ptrlib
- ROP gadget finder
    - ropr
    - rp++
