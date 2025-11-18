# pwnenv
CTF の pwn 用の個人的な docker 環境


## 使い方
`Makefile` を `ctf/Makefile` に配置するとすると、
`Dockerfile` は `ctf/ctfenv/pwnenv/Dockerfile` に配置し、
それぞれの問題のディレクトリは `ctf/<ctf-site-name>/<problem-name>` に配置する。


`make init` によって必要なファイルが用意され、設定がなされる。
`make run` によってコンテナが開始する。


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
