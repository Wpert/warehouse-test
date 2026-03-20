.PHONY: all build clean rebuild status deps

GREEN = \033[0;32m
NC = \033[0m

all: build

build:
	@echo "$(GREEN)▶ Сборка проекта...$(NC)"
	@scripts/build.sh

rebuild:
	@echo "$(GREEN)▶ Пересборка проекта...$(NC)"
	@scripts/rebuild.sh

clean:
	@echo "$(GREEN)▶ Очистка...$(NC)"
	rm -rf build/

status:
	@echo "$(GREEN)▶ Статус:$(NC)"
	@if [ -d "build/bin" ]; then \
		echo "Программа: build/bin/warehouse"; \
		ls -la build/bin/warehouse; \
	else \
		echo "Программа не собрана"; \
	fi

run: build
	@./build/bin/warehouse

help:
	@echo "Доступные команды:"
	@echo "  make build    - Собрать проект"
	@echo "  make rebuild  - Пересобрать с нуля"
	@echo "  make clean    - Очистить build/"
	@echo "  make status   - Статус сборки"
	@echo "  make run      - Запустить программу"
