@echo off
setlocal

if not defined MSYS2_ROOT set "MSYS2_ROOT=C:\code\msys2"
if not exist "%MSYS2_ROOT%\usr\bin\bash.exe" (
    if exist "C:\msys64\usr\bin\bash.exe" set "MSYS2_ROOT=C:\msys64"
)
if not exist "%MSYS2_ROOT%\usr\bin\bash.exe" (
    echo ERROR: no se encuentra MSYS2 en "%MSYS2_ROOT%".
    echo Instala MSYS2 ahi o define la variable MSYS2_ROOT.
    exit /b 1
)

echo === Instalando dependencias MinGW-w64 ===
echo MSYS2: "%MSYS2_ROOT%"
echo Paquetes: gcc, libgomp, ninja, SDL2, pkgconf
echo.

"%MSYS2_ROOT%\usr\bin\bash.exe" -lc "pacman -Sy --noconfirm && pacman -S --needed --noconfirm mingw-w64-x86_64-gcc mingw-w64-x86_64-libgomp mingw-w64-x86_64-ninja mingw-w64-x86_64-SDL2 mingw-w64-x86_64-pkgconf"
if errorlevel 1 goto :error

set "MINGW_BIN=%MSYS2_ROOT%\mingw64\bin"
set "MINGW_PREFIX=%MSYS2_ROOT%\mingw64"
if not exist "%MINGW_BIN%\gcc.exe" goto :incomplete
if not exist "%MINGW_BIN%\ninja.exe" goto :incomplete
if not exist "%MINGW_BIN%\SDL2.dll" goto :incomplete
if not exist "%MINGW_PREFIX%\lib\cmake\SDL2\SDL2Config.cmake" goto :incomplete
if not exist "%MINGW_BIN%\libgomp-1.dll" goto :incomplete

call :resolve_cmake
if not defined CMAKE_EXE goto :missing_cmake

echo.
echo Dependencias listas.
echo   gcc:   "%MINGW_BIN%\gcc.exe"
echo   ninja: "%MINGW_BIN%\ninja.exe"
echo   SDL2:  "%MINGW_PREFIX%\lib\cmake\SDL2\SDL2Config.cmake"
echo   cmake: "%CMAKE_EXE%"
exit /b 0

:resolve_cmake
set "CMAKE_EXE="
where cmake >nul 2>&1
if not errorlevel 1 (
    for /f "delims=" %%I in ('where cmake 2^>nul') do (
        set "CMAKE_EXE=%%I"
        goto :resolve_cmake_done
    )
)
for /d %%D in (C:\code\cmake-*) do (
    if exist "%%D\bin\cmake.exe" (
        set "CMAKE_EXE=%%D\bin\cmake.exe"
        goto :resolve_cmake_done
    )
)
goto :resolve_cmake_done

:resolve_cmake_done
exit /b 0

:missing_cmake
echo.
echo ERROR: no se encuentra CMake.
echo Se espera cmake.exe en el PATH o en C:\code\cmake-*\bin.
exit /b 1

:incomplete
echo.
echo ERROR: pacman termino, pero falta alguna herramienta en "%MSYS2_ROOT%\mingw64".
exit /b 1

:error
echo.
echo ERROR: la instalacion de dependencias ha fallado.
exit /b 1
