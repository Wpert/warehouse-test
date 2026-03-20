#!/bin/bash
set -e

# Цвета для вывода
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}===== Запуск тестов с покрытием кода =====${NC}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

# Проверяем наличие gcov/lcov
if ! command -v lcov &> /dev/null; then
    echo -e "${YELLOW}Устанавливаем lcov для анализа покрытия...${NC}"
    if command -v apt-get &> /dev/null; then
        sudo apt-get install -y lcov
    elif command -v brew &> /dev/null; then
        brew install lcov
    else
        echo -e "${RED}Не удалось установить lcov. Установите вручную.${NC}"
        exit 1
    fi
fi

# Проверяем существование build
if [ ! -d "build" ]; then
    echo -e "${YELLOW}Папка build не найдена. Создаем с поддержкой покрытия...${NC}"
    mkdir -p build
    cd build
    
    # Устанавливаем зависимости
    conan install .. --build=missing
    
    # Конфигурация с флагами покрытия
    cmake .. -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="--coverage -fprofile-arcs -ftest-coverage"
    
    cd "$PROJECT_ROOT"
fi

cd build

# Сборка с флагами покрытия
echo -e "${YELLOW}Сборка с флагами покрытия...${NC}"
make -j4

# Запуск тестов
echo -e "${YELLOW}Запуск тестов...${NC}"
ctest --output-on-failure

# Сбор информации о покрытии
echo -e "${YELLOW}Сбор информации о покрытии...${NC}"
lcov --capture --directory . --output-file coverage.info --rc lcov_branch_coverage=1 2>/dev/null || true
lcov --remove coverage.info '/usr/*' '*/tests/*' '*/build/*' '/usr/include/*' '*/_deps/*' --output-file coverage_filtered.info --rc lcov_branch_coverage=1 2>/dev/null || true

# Генерация HTML отчета
echo -e "${YELLOW}Генерация HTML отчета...${NC}"
genhtml coverage_filtered.info --output-directory coverage_report --branch-coverage 2>/dev/null || true

echo -e "${GREEN}Отчет о покрытии создан в build/coverage_report/${NC}"
echo -e "${GREEN}Откройте build/coverage_report/index.html в браузере${NC}"

cd "$PROJECT_ROOT"
