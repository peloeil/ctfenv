# pwnenv
CTF の pwn 用の個人的な docker 環境

## 前提
- `Makefile` を `ctf/Makefile` に配置すると仮定したときの各ファイルの配置
  - `Dockerfile`: `ctf/ctfenv/pwnenv/Dockerfile`
  - それぞれの問題のディレクトリ: `ctf/<ctf-site-name>/<problem-name>`
- `ctf/<ctf-site-name>/<problem-name>` には以下があることを仮定
  - 問題バイナリ
  - `libc.so.6`
  - `Dockerfile`
  - compose ファイル

## 使い方
`make init PROBLEM_PATH=<ctf-site-name>/<problem-name> BINARY_NAME=<binary-name>` によって必要なファイルが用意され、設定がなされる。
`make run PROBLEM_PATH=<ctf-site-name>/<problem-name> BINARY_NAME=<binary-name>` によってコンテナが開始する。


問題によって port は調整する必要がある。

コンテナ内の変更は `exploit.py` のみが保存される。

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
