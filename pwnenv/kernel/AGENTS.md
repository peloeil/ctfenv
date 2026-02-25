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

## `src/exploit.c` Implementation Rules
- カーネルモジュールが提供される場合は、`src/exploit.c` に問題固有のラッパーを実装する。
  - `ioctl` ラッパーを作る場合、第三引数は呼び出し側から渡さずラッパー内で構築する。
- 処理の追跡性を優先し、原則として関数分割を最小化して `main` に exploit phase を並べる。
  - 新たに関数を定義する場合は、プロトタイプ宣言を行い、実装は `main` の下に配置する。
- マクロ定義は spray 用を優先し、他用途のマクロは必要最小限にする。
  - spray 回数を表すマクロ名は `NUM_*_SPRAY` 形式に統一する（例: `NUM_PTE_SPRAY`）。
  - `kmem_cache_cpu` 周辺の値は `OBJS_PER_SLAB` のように対象を連想しやすい命名にする。
  - `ioctl` の `cmd` を表すマクロは可読性のために定義してよい。
  - `ioctl` `cmd` など exploit 本体に直接関わらない補助マクロは、原則として `main` の下に配置する。
- 変数は「本当に必要なものだけ」を「必要な直前」で宣言する。
  - 可能な限り `const` を付ける。
- 型はサイズが明示的なものを使う。
  - `int` / `long` / `size_t` などは原則避け、64bit 環境前提で `i32` / `u64` などを使う。
- 返り値チェックは `util.h` の `CHECK*` マクロを使えるかを最初に確認する。
- 二重 `for` などで分岐が増えて追いにくい場合、phase 境界を明確にするため `goto` を許容する。

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
