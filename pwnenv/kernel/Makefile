# Exclude sources from build (space-separated paths or patterns).
EXCLUDE_SRCS ?=

SRCS := $(filter-out $(EXCLUDE_SRCS), $(wildcard src/*.c *.c))
ASMS := $(patsubst %.c, %.s, $(SRCS))
CFLAGS ?=

# デフォルトのビルドターゲットを指定 (変更したい場合は make run PACKER=pack-gcc とする)
PACKER ?= pack

.PHONY: enter pack pack-gcc run submit

enter:
	@# pwnenv サービスが起動しているか確認し、していない場合のみ up する
	@if [ -z "$$(docker compose ps -q pwnenv)" ]; then \
		echo "Starting container..."; \
		docker compose up -d --build; \
	fi
	docker compose exec pwnenv /bin/fish

pack: $(SRCS)
	musl-gcc -static $(CFLAGS) $(SRCS) -o rootdir/exploit
	cd rootdir; find . -print0 | cpio -o --format=newc --null --owner=root > ../rootfs_dev.cpio; cd ..

%.s: %.c
	gcc -S $(CFLAGS) $< -o $@

pack-gcc: $(ASMS)
	musl-gcc -static $(CFLAGS) $(ASMS) -o rootdir/exploit
	cd rootdir; find . -print0 | cpio -o --format=newc --null --owner=root > ../rootfs_dev.cpio; cd ..

# 依存先を $(PACKER) 変数に変更
run: $(PACKER)
	./dev.sh

submit: CFLAGS := -Os -flto -fno-plt -fdata-sections -ffunction-sections -fno-stack-protector -fomit-frame-pointer -fmerge-all-constants -Wl,--gc-sections
submit: $(PACKER)
	strip -s ./rootdir/exploit
	upx ./rootdir/exploit
	./upload.py
