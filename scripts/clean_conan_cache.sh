#!/bin/bash
set -e

YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}===== Очистка кэша Conan =====${NC}"
read -p "Очистить кэш Conan? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    conan remove "*" -f
    echo "✅ Кэш очищен"
else
    echo "Отменено"
fi
