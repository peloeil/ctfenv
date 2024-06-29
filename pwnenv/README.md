# pwnenv
CTF の pwn 用の一般的な docker 環境

## 必要なもの
- Linux
- Docker
- docker-compose

ArchLinux の場合
```
# docker, docker-compose のインストール
sudo pacman -S docker docker-compose

# docker コマンドを sudo なしで実行できるように自身を docker グループに追加
sudo usermod -aG docker $(id -un)

# 上の変更を適用するために再起動
sudo reboot
```

## 使い方
- 初めに、ユーザー名などを設定するために `set_dotenv.sh` を実行する
- コンテナの起動: `docker compose up -d`
- コンテナに接続: `docker exec -it pwnenv /bin/bash`
- コンテナから出る: `Ctrl-D`
- コンテナを停止: `docker compose stop`
- コンテナを削除: `docker compose down`
- `workdir` の中身のみが保存され、その他の変更はコンテナ削除時に一緒に消える

## 詳細
- テキストエディタは nano, vi, vim, neovim が用意されている
