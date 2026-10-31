@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"
set "BUILD_DIR=%SCRIPT_DIR%\build"

if not defined MSYS2_ROOT set "MSYS2_ROOT=C:\code\msys2"
if not exist "%MSYS2_ROOT%\mingw64\bin\gcc.exe" (
    if exist "C:\msys64\mingw64\bin\gcc.exe" set "MSYS2_ROOT=C:\msys64"
)
set "MINGW_PREFIX=%MSYS2_ROOT%\mingw64"
set "MINGW_BIN=%MINGW_PREFIX%\bin"

if not exist "%MINGW_BIN%\gcc.exe" goto :missing_deps
if not exist "%MINGW_BIN%\ninja.exe" goto :missing_deps
if not exist "%MINGW_PREFIX%\lib\cmake\SDL2\SDL2Config.cmake" goto :missing_deps

call :resolve_cmake
if not defined CMAKE_EXE goto :missing_cmake

set "PATH=%MINGW_BIN%;%CMAKE_BIN%;%PATH%"
set "CMAKE_PREFIX_PATH=%MINGW_PREFIX%"
set "MINGW_PREFIX_CMAKE=%MINGW_PREFIX:\=/%"
set "MINGW_GCC_CMAKE=%MINGW_BIN:\=/%/gcc.exe"

echo === Configurando fractus-x64 ===
echo CMake: "%CMAKE_EXE%"
echo GCC:   "%MINGW_BIN%\gcc.exe"
echo SDL2:  "%MINGW_PREFIX%"
echo.

"%CMAKE_EXE%" -S "%SCRIPT_DIR%" -B "%BUILD_DIR%" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="%MINGW_GCC_CMAKE%" -DCMAKE_PREFIX_PATH="%MINGW_PREFIX_CMAKE%"
if errorlevel 1 goto :error

echo.
echo === Compilando fractus-x64 ===
"%CMAKE_EXE%" --build "%BUILD_DIR%"
if errorlevel 1 goto :error

echo.
echo Compilacion terminada.
echo Ejecutable: "%BUILD_DIR%\src\fractus_x64.exe"
goto :end

:resolve_cmake
set "CMAKE_EXE="
set "CMAKE_BIN="
where cmake >nul 2>&1
if not errorlevel 1 (
    for /f "delims=" %%I in ('where cmake 2^>nul') do (
        set "CMAKE_EXE=%%I"
        goto :resolve_cmake_bin
    )
)
for /d %%D in (C:\code\cmake-*) do (
    if exist "%%D\bin\cmake.exe" (
        set "CMAKE_EXE=%%D\bin\cmake.exe"
        set "CMAKE_BIN=%%D\bin"
        goto :resolve_cmake_done
    )
)
goto :resolve_cmake_done

:resolve_cmake_bin
for %%I in ("%CMAKE_EXE%") do set "CMAKE_BIN=%%~dpI"
if "%CMAKE_BIN:~-1%"=="\" set "CMAKE_BIN=%CMAKE_BIN:~0,-1%"

:resolve_cmake_done
exit /b 0

:missing_deps
echo ERROR: faltan GCC, Ninja o SDL2 de MinGW-w64.
echo Ejecuta install-deps.bat en la raiz del proyecto.
echo MSYS2 buscado en "%MSYS2_ROOT%".
exit /b 1

:missing_cmake
echo ERROR: no se encuentra CMake.
echo Se espera cmake.exe en el PATH o en C:\code\cmake-*\bin.
exit /b 1

:error
echo.
echo ERROR: la compilacion ha fallado.
exit /b 1

:end
endlocal
