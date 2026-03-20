#!/bin/bash
set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}===== Полная пересборка Warehouse =====${NC}"

# Полная очистка
rm -rf build

# Запуск сборки
./scripts/build.sh
