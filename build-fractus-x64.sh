#!/usr/bin/env sh
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "=== Configurando fractus-x64 ==="
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=Release

echo
echo "=== Compilando fractus-x64 ==="
cmake --build "$BUILD_DIR"

echo
echo "Compilacion terminada."
echo "Ejecutable esperado en una de estas rutas:"
echo "  $BUILD_DIR/src/fractus-x64.exe"
echo "  $BUILD_DIR/src/Release/fractus-x64.exe"
