.PHONY: enter run

enter:
	@# pwnenv サービスが起動しているか確認し、していない場合のみ up する
	@if [ -z "$$(docker compose ps -q pwnenv)" ]; then \
		echo "Starting container..."; \
		docker compose up -d --build; \
	fi
	docker compose exec pwnenv /bin/fish

run:
	./exploit.py
