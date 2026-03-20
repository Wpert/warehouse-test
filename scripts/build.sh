#!/bin/bash
set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}===== Сборка Warehouse с Conan 2 =====${NC}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

# Проверка наличия Conan
if ! command -v conan &> /dev/null; then
    echo -e "${RED}Conan не найден. Установите: pipx install conan${NC}"
    exit 1
fi

echo -e "${GREEN}Conan версия: $(conan --version)${NC}"

# Очистка
rm -rf build
mkdir build
cd build

# Установка зависимостей через Conan 2
echo -e "${YELLOW}[1/3] Установка зависимостей Conan...${NC}"
conan install .. --output-folder=. --build=missing

# Конфигурация CMake
echo -e "${YELLOW}[2/3] Конфигурация CMake...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake

# Сборка
echo -e "${YELLOW}[3/3] Сборка проекта...${NC}"
cmake --build . --config Release --parallel

# Проверка результата
if [ -f "bin/warehouse" ]; then
    echo -e "${GREEN}Готово! Программа: build/bin/warehouse${NC}"
else
    echo -e "${RED}Ошибка сборки${NC}"
    exit 1
fi

cd "$PROJECT_ROOT"
