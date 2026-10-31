#!/usr/bin/env sh
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

if [ -z "${MSYS2_ROOT:-}" ] || [ ! -d "$MSYS2_ROOT/mingw64/bin" ]; then
    if [ -d /c/code/msys2/mingw64/bin ]; then
        MSYS2_ROOT=/c/code/msys2
    fi
fi

if [ -n "${MSYS2_ROOT:-}" ] && [ -d "$MSYS2_ROOT/mingw64/bin" ]; then
    export PATH="$MSYS2_ROOT/mingw64/bin:$PATH"
    export CMAKE_PREFIX_PATH="$MSYS2_ROOT/mingw64${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
fi

if ! command -v cmake >/dev/null 2>&1; then
    for cmake_dir in /c/code/cmake-*/bin; do
        if [ -x "$cmake_dir/cmake.exe" ]; then
            export PATH="$cmake_dir:$PATH"
            break
        fi
    done
fi

missing=0
if ! command -v cmake >/dev/null 2>&1; then
    echo "ERROR: no se encuentra CMake. Se espera cmake en el PATH o en /c/code/cmake-*/bin."
    missing=1
fi
if ! command -v gcc >/dev/null 2>&1; then
    echo "ERROR: no se encuentra gcc. En Windows ejecuta install-deps.bat."
    missing=1
fi
if ! command -v ninja >/dev/null 2>&1; then
    echo "ERROR: no se encuentra ninja. En Windows ejecuta install-deps.bat."
    missing=1
fi
if [ "$missing" -ne 0 ]; then
    exit 1
fi

CMAKE_ARGS="-G Ninja -DCMAKE_BUILD_TYPE=Release"
if [ -n "${MSYS2_ROOT:-}" ] && [ -x "$MSYS2_ROOT/mingw64/bin/gcc.exe" ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_C_COMPILER=$MSYS2_ROOT/mingw64/bin/gcc.exe -DCMAKE_PREFIX_PATH=$MSYS2_ROOT/mingw64"
fi

echo "=== Configurando fractus-x64 ==="
# shellcheck disable=SC2086
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" $CMAKE_ARGS

echo
echo "=== Compilando fractus-x64 ==="
cmake --build "$BUILD_DIR"

echo
echo "Compilacion terminada."
echo "Ejecutable: $BUILD_DIR/src/fractus_x64.exe"
