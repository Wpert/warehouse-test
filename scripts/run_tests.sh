#!/bin/bash
set -e

# возможно теперь скрипт не будет работать так как внёс некоторые изменения в cmake
# нужный функционал проверил на этапе разработки, сейчас полирую приложение

# Цвета для вывода
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}===== Запуск тестов Warehouse =====${NC}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

# Проверяем, существует ли папка build с зависимостями
if [ ! -d "build" ] || [ ! -f "build/conanbuildinfo.cmake" ]; then
    echo -e "${YELLOW}Папка build не найдена или отсутствуют зависимости Conan.${NC}"
    echo -e "${YELLOW}Запускаем полную сборку с тестами...${NC}"
    
    # Создаем папку build
    mkdir -p build
    cd build
    
    # Устанавливаем зависимости через Conan
    echo -e "${YELLOW}Установка зависимостей через Conan...${NC}"
    conan install .. --build=missing
    
    # Конфигурация CMake с тестами
    echo -e "${YELLOW}Конфигурация CMake...${NC}"
    cmake .. -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
    
    cd "$PROJECT_ROOT"
else
    echo -e "${YELLOW}Папка build существует, обновляем конфигурацию для тестов...${NC}"
    cd build
    
    # Проверяем, включены ли тесты в текущей конфигурации
    if ! cmake -L 2>/dev/null | grep -q "BUILD_TESTS:BOOL=ON"; then
        echo -e "${YELLOW}Тесты не были включены. Перенастраиваем...${NC}"
        cmake .. -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
    fi
    
    cd "$PROJECT_ROOT"
fi

# Сборка тестов
echo -e "${YELLOW}Сборка тестов...${NC}"
cd build
make -j4

# Запуск тестов
echo -e "${YELLOW}Запуск тестов...${NC}"
ctest --output-on-failure --verbose

echo -e "${GREEN}===== Тесты завершены =====${NC}"
cd "$PROJECT_ROOT"
