EXCLUDE_SRCS ?= ./src/template/fuse_example.c
SRCS := $(filter-out $(EXCLUDE_SRCS), $(shell find ./src -name "*.c"))
LIBS ?=

ifneq ($(strip $(LIBS)),)
	CC := gcc
	CFLAGS := -static
	MSG := "Compiling with GCC (Libraries detected)..."
else
	CC := musl-gcc
	CFLAGS := -static -idirafter /usr/include/x86_64-linux-gnu -idirafter /usr/include
	MSG := "Compiling with musl-gcc (Size optimized)..."
endif

.PHONY: enter pack run submit

enter:
	@if [ -z "$$(docker compose ps --quiet pwnenv)" ]; then \
		echo "Starting container..."; \
		docker compose up --detach --build; \
	fi
	docker compose exec pwnenv /bin/fish

pack:
	@echo $(MSG)
	$(CC) $(CFLAGS) $(SRCS) -o rootdir/exploit $(LIBS)
	cd rootdir; find . -print0 | cpio --create --format=newc --null --owner=root > ../rootfs_dev.cpio; cd ..

run: pack
	./dev.sh

# 圧縮するためのオプションで挙動が変わる可能性がある
# remote でうまく行かない場合は以下を試す
#submit: pack
#	./upload.py
submit: CFLAGS += -Os \
					-fno-ident \
					-fno-plt \
					-fno-stack-protector \
					-fomit-frame-pointer \
					-fno-unwind-tables \
					-fno-asynchronous-unwind-tables \
					-fno-exceptions \
					-fmerge-all-constants \
					-falign-functions=1 \
					-falign-jumps=1 \
					-falign-loops=1 \
					-falign-labels=1 \
					-fdata-sections \
					-ffunction-sections \
					-Wl,--gc-sections \
					-Wl,--build-id=none
submit: pack
	strip --strip-all \
			--remove-section=.comment \
			--remove-section=.note.ABI-tag \
			--remove-section=.note.gnu.build-id \
			./rootdir/exploit
	upx --best --ultra-brute ./rootdir/exploit
	./upload.py
