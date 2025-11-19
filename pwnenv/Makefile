PROBLEM_PATH ?= default_path
BINARY_NAME  ?= binary

BINARY_PATH := $(PROBLEM_PATH)/$(BINARY_NAME)

COMPOSE_CANDIDATES := compose.yml compose.yaml docker-compose.yml docker-compose.yaml
DOCKERFILE         := $(firstword $(wildcard $(PROBLEM_PATH)/Dockerfile))
COMPOSE_FILE       := $(firstword $(foreach f,$(COMPOSE_CANDIDATES),$(wildcard $(PROBLEM_PATH)/$(f))))

.PHONY: validate init run

validate:
	@if [ "$(PROBLEM_PATH)" = "default_path" ]; then \
		echo "please set PROBLEM_PATH"; \
		exit 1; \
	fi
	@if [ "$(BINARY_NAME)" = "binary" ]; then \
		echo "please set BINARY_NAME"; \
		exit 1; \
	fi
	@if [ -z "$(DOCKERFILE)" ]; then \
		echo "Dockerfile not found under $(PROBLEM_PATH)"; \
		exit 1; \
	fi
	@if [ -z "$(COMPOSE_FILE)" ]; then \
		echo "compose file (compose.yaml/docker-compose.*) not found under $(PROBLEM_PATH)"; \
		exit 1; \
	fi
	@if [ -f "$(PROBLEM_PATH)/exploit.py" ]; then \
		echo "exploit.py already exists"; \
		exit 1; \
	fi
	@echo "[+] PROBLEM_PATH = $(PROBLEM_PATH)"
	@echo "[+] BINARY_NAME  = $(BINARY_NAME)"
	@echo "[+] dockerfile   = $(DOCKERFILE)"
	@echo "[+] compose file = $(COMPOSE_FILE)"

init: validate
	@echo "[+] running pwninit in $(PROBLEM_PATH)"
	@docker image build \
		--file ctfenv/pwnenv/pwninit.Dockerfile \
		--build-arg PROBLEM_PATH="$(PROBLEM_PATH)" \
		--build-arg BINARY_NAME="$(BINARY_NAME)" \
		--output type=local,dest="$(PROBLEM_PATH)" \
		.
	@mv $(PROBLEM_PATH)/ld-* "$(PROBLEM_PATH)/ld-linux-x86-64.so.2"

	@echo "[+] generating attach.sh / gdb.py / exploit.py under $(PROBLEM_PATH)"

	@printf '%s\n' \
		'PID=$$(pgrep $(BINARY_NAME) | tail -n 1)' \
		'echo "$$PID"' \
		'sudo MY_PID=$$PID gdb -x gdb.py' \
		> "$(PROBLEM_PATH)/attach.sh"
	@chmod +x "$(PROBLEM_PATH)/attach.sh"

	@printf '%s\n' \
		'import gdb, os' \
		'' \
		'pid = os.environ.get("MY_PID")' \
		'' \
		'gdb.execute("set sysroot")' \
		'gdb.execute("file /home/pwn/$(BINARY_NAME)")' \
		'gdb.execute(f"attach {pid}")' \
		> "$(PROBLEM_PATH)/gdb.py"

	@printf '%s\n' \
		'#!/usr/bin/env -S uv run' \
		'# /// script' \
		'# dependencies = [' \
		'#     "ptrlib",' \
		'#     "pwntools",' \
		'# ]' \
		'# ///' \
		'from ptrlib import *' \
		'' \
		'libc = ELF("/lib/x86_64-linux-gnu/libc.so.6")' \
		'elf = ELF("/home/pwn/$(BINARY_NAME)")' \
		'io = Socket("localhost", 9999)' \
		'' \
		'' \
		'' \
		'io.interactive()' \
		> "$(PROBLEM_PATH)/exploit.py"
	@chmod +x "$(PROBLEM_PATH)/exploit.py"

	@SERVICE_NAME=$$(awk ' \
		/^services:/ {in_services=1; next} \
		in_services && /^[^[:space:]]/ {in_services=0} \
		in_services && /^[[:space:]][[:space:]]+[A-Za-z0-9_-]+:/ { \
			gsub(":", "", $$1); gsub(" ", "", $$1); print $$1; exit \
		}' "$(COMPOSE_FILE)"); \
	echo "[+] service name: $$SERVICE_NAME"; \
	printf '%s\n' \
		'' \
		'  pwnenv:' \
		'    build:' \
		'      context: ../..' \
		'      dockerfile: ctfenv/pwnenv/Dockerfile' \
		'      args:' \
		"        PROBLEM_PATH: \"$(PROBLEM_PATH)\"" \
		"        BINARY_NAME: \"$(BINARY_NAME)\"" \
		"    pid: \"service:$$SERVICE_NAME\"" \
		'    cap_add:' \
		'      - "SYS_PTRACE"' \
		'    security_opt:' \
		'      - "seccomp:unconfined"' \
		'    depends_on:' \
		"      - \"$$SERVICE_NAME\"" \
		'    stdin_open: true' \
		'    tty: true' \
		'    volumes:' \
		'      - ./exploit.py:/home/pwn/exploit.py:rw' \
		>> "$(COMPOSE_FILE)"

run:
	@cd "$(PROBLEM_PATH)" && docker compose up -d --build
	@cd "$(PROBLEM_PATH)" && docker compose exec pwnenv fish
