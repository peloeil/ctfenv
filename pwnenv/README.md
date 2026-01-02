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

## 説明
- `penv init (bare|stack|heap|kernel)`: その問題における環境構築を行う。
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
            - 問題バイナリ, `libc.so.6`, ld, `exploit.py` はすべて `/home/pwn` にマウントされる
        - `compose.yaml` がなければ exploit 用の `compose.yaml` を作る
    - heap
        - libc がない場合は問題用コンテナから libc を抽出する
        - pwninit で libc を unstrip し、対応する ld も用意する
        - Dockerfile が無い場合は stack 問題として扱う
        - `gdb.py`, `attach.sh`, `exploit.py` を用意する
        - デバッグシンボル付き libc のファイル名を`libc.so.6` に統一する
        - compose file の名前を `compose.yaml` に統一する
        - `compose.yaml` に問題用コンテナと exploit 用コンテナを一緒に起動する設定を追加する
            - exploit 用コンテナでは `nc <problem-service-name> <port-number>` で問題用コンテナに接続できる
            - 問題バイナリ, `libc.so.6`, ld, `gdb.py`, `attach.sh`, `exploit.py` はすべて `/home/pwn` にマウントされる
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
        - `compose.yaml` がなければ exploit 用の `compose.yaml` を作る
## 注意
- malloc, free の仕様を使う問題であっても、Dockerfile と compose file が配布されていない場合 stack 問題として扱う
- heap 問題として扱うのは注意が必要
    - stack 問題として扱っても、バイナリはデバッグシンボル付き libc に patchelf されているため、libc 内部の挙動やシンボル解決は問題ない
    - exploit 用コンテナの libc, ld を置き換えるため不安定な可能性がある
    - libc の配置されるアドレスのアラインメントがホストのファイルシステムに一部依存したりするため、純粋に問題サーバーの環境と合わせたい場合のみ heap 問題として扱う
