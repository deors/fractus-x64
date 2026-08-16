@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"
set "BUILD_DIR=%SCRIPT_DIR%\build"

echo === Configurando fractus-x64 ===
cmake -S "%SCRIPT_DIR%" -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto :error

echo.
echo === Compilando fractus-x64 ===
cmake --build "%BUILD_DIR%" --config Release
if errorlevel 1 goto :error

echo.
echo Compilacion terminada.
echo Ejecutable esperado en una de estas rutas:
echo   "%BUILD_DIR%\src\fractus-x64.exe"
echo   "%BUILD_DIR%\src\Release\fractus-x64.exe"
goto :end

:error
echo.
echo ERROR: la compilacion ha fallado.
exit /b 1

:end
endlocal
