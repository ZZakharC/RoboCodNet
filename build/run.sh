#!/bin/bash

# Директории
SRC_DIR=./src
BUILD_DIR=./build
BIN_DIR="$BUILD_DIR/bin"

# Информация 
VERSION="1.0.1"
NAME="roboCodNetBot"

# Компилятор
MAX_JOBS=$(nproc) # Максимальное количество параллельных компиляций (nproc - количество ядер CPU)
CC="g++" # GCC можно Clang
LD="g++"
FLAGS_CC="-std=c++20 -Wall -Wextra -Wpedantic -O3 -flto -march=native -funroll-loops -I./include -I./src"
FLAGS_LD="-std=c++20 -flto=8 -fuse-linker-plugin -L/usr/local/lib -lTgBot -lssl -lsqlite3 -lcrypto -lpthread"
PATH_BIN=$BIN_DIR"/"$NAME

JOBS=0 # Счётчик запущены компиляторов  

# Цвета
GREEN='\033[0;32m'
BLUE='\033[0;36m'
PURPLE='\033[0;95m'
NC='\033[0m'

# Очистка
clear -x

mkdir -p $BIN_DIR

# === Компиляция ===
C_FILES=$(find "$SRC_DIR" -name "*.cpp")
OBJ_FILES=""

for file in $C_FILES; do
    # заменяем .cpp -> .o и добавляем BIN_DIR
    obj="$BIN_DIR/${file#$SRC_DIR%}.o"

    # создаём подкаталоги при необходимости
    mkdir -p "$(dirname "$obj")"

    # Компилируем если есть изменения 
    if [[ ! -f "$obj" || "$file" -nt "$obj" ]]; then
        echo -e "${BLUE}Компиляция: $file -> $obj${NC}"
        $CC $FLAGS_CC -c "$file" -o "$obj" &

        ((JOBS++))
        if [[ $JOBS -ge $MAX_JOBS ]]; then
            wait
            JOBS=0
        fi
    fi

    OBJ_FILES="$OBJ_FILES $obj"
done

# Ждём завершения ВСЕХ фоновых компиляций
wait

echo -e "${PURPLE}Сборка -> $PATH_BIN${NC}"
$LD $OBJ_FILES $FLAGS_LD -o $PATH_BIN

# === Старт ===
echo -e "${GREEN}Программа успешно собрана и находится по адресу \"${PATH_BIN}\" ${NC}"

clear -x