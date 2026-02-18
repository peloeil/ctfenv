# Kernel Exploit Instructions

## Scope
- この指示は「各 CTF の kernel exploit 問題を解くとき」に使う。
- 前提は `penv init kernel` 後の問題ディレクトリ構成:
  - ルート: `Makefile`, `vmlinux`, `run.sh`, `dev.sh`, `rootfs*`, `rootdir/`, `gdb.py`, `upload.py`
  - 実装本体: `src/exploit.c`（`main` を含む）
  - 補助テンプレート: `src/template/`

## Command Policy
- コマンドは `fish -c '<command>'` 経由で実行する。
- 提案コマンドも `fish -c` 形式を優先する。

## Development Policy
- まず問題固有情報を優先して読む:
  - 配布モジュール/ドライバ、`run.sh`、rootfs 内容、`vmlinux`、ログ（dmesg / gdb）
- `src/template/` は汎用補助であり、脆弱性固有ロジックではない前提で扱う。
- exploit の主ロジックは `src/exploit.c` に実装する。
- 使わないテンプレート依存は入れない。
- 問題固有の値・分岐・アドレス管理は `src/exploit.c` 側に寄せる。

## Build/Run Facts
- `make run` は `compile -> pack -> ./dev.sh` を自動で実行する。
- `make compile` / `make pack` は切り分けデバッグ時に個別利用する。
- `Makefile` は原則 `./src/**/*.c` をコンパイル対象にする（デフォルト除外あり）。

## Exploit Workflow
- 段階的に進め、各段階で再現可能にする:
  1. バグトリガを安定化（crash/UAF/OOB など）
  2. 使える primitive（AAR/AAW/PC control など）を確立
  3. `uid=0` 取得とシェル/flag 動線を完成
- 緩和策（KASLR/SMEP/SMAP/KPTI/CET など）を明示し、方針に反映する。
- ハードコード値を使う場合は導出根拠を残す。

## Template Invariants
- 変更対象の template は原則 `src/template/vars.h` のみ。
- `vars.h` でも変更対象は `/* editable */` として用意された項目のみ。
- 具体的には、問題に応じて以下を更新してよい:
  - `CONFIG_*` 系マクロ
  - `addr_*` / gadget 系マクロ（例: `addr_commit_creds` など）
  - `INIT_*` 系マクロ
  - `kaslr` の初期方針（`INIT_KASLR` または exploit 側での運用）
- `vars.h` の `/* uneditable */` 領域、および他の `src/template/*` は原則変更しない。

## Preserve Challenge Assets
- `run.sh` と配布された `rootfs.*` は変更しない（例外なし）。
- 開発・検証には `dev.sh` と `rootfs_dev.*` を使う。

## Reporting
- 作業報告には必ず以下を含める:
  - 変更ファイル
  - 問題固有で必要な理由
  - 再現コマンド
  - 成功率と既知の失敗パターン
