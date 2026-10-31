# fractus-x64

Port a x64 de Fractus. Fractus en una suite de generación de conjuntos fractales que realicé en el año 1997 sobre Turbo C para MS-DOS 16-bits en CPUs 386 y 486, y modos de vídeo VGA y SVGA (modos de 256 colores desde 320x200 hasta 1280x1024).

## Estructura inicial

- `src/app`: arranque y composición de la aplicación.
- `src/core`: lógica portable y algoritmos.
- `src/ui`: interfaz y comportamiento visual en coordenadas lógicas.
- `src/platform`: integración con la plataforma y futuras librerías base.
- `paletas`: ficheros de configuración de paletas de colores.
- `fractus.fon`: fuentes de letra usadas en la interfaz gráfica.
- `fractus.cfg`: configuración persistente.

El código original del proyecto permanece intacto fuera de esta carpeta.

## Build

Este árbol es el port moderno:

- lenguaje: **C11**
- sistema de build: **CMake** y **Ninja**
- compilador en Windows: **GCC de MinGW-w64** (MSYS2)
- backend de ventana: **SDL2**
- paralelismo del cálculo: **OpenMP**

### Requisitos

Deben estar instalados antes de compilar:

- **MSYS2**, con la variable `MSYS2_ROOT` apuntando a la raíz de la instalación. Ese directorio contiene `usr\bin\bash.exe`.
- **CMake**, disponible en el `PATH`.

`install-deps.bat` instala en el entorno `mingw64` de ese MSYS2, con `pacman -S --needed`, los paquetes que el build usa y que no vienen con el sistema base:

| Paquete | Para qué |
|---|---|
| `mingw-w64-x86_64-gcc` | compilador C11 |
| `mingw-w64-x86_64-libgomp` | OpenMP |
| `mingw-w64-x86_64-ninja` | generador de CMake |
| `mingw-w64-x86_64-SDL2` | ventana, eventos y presentación |
| `mingw-w64-x86_64-pkgconf` | localizar dependencias desde CMake |

```powershell
.\install-deps.bat
```

El script se puede repetir. Pacman no vuelve a descargar un paquete que ya está instalado. CMake no entra en esa lista: tiene que estar ya en el `PATH`.

### Compilar y ejecutar

`build-fractus-x64.bat` localiza MinGW con `MSYS2_ROOT` y CMake con el `PATH`. Antepone `%MSYS2_ROOT%\mingw64\bin` al `PATH`, define `CMAKE_PREFIX_PATH` con ese prefijo para que CMake encuentre SDL2 y compila en Release con Ninja.

```powershell
.\build-fractus-x64.bat
.\run-fractus-x64.bat
```

El ejecutable queda en `build\src\fractus_x64.exe`. El build copia a su lado `SDL2.dll`, `libgomp-1.dll`, `libwinpthread-1.dll` y `libgcc_s_seh-1.dll`.

Los tests enlazan SDL2, así que el `bin` de MinGW tiene que estar en el `PATH`:

```powershell
$env:PATH = "$env:MSYS2_ROOT\mingw64\bin;" + $env:PATH
ctest --test-dir build --output-on-failure
```

En Linux, o dentro de un shell de MSYS2, `build-fractus-x64.sh` hace el mismo build. Usa `MSYS2_ROOT` si está definida; si no, espera `cmake`, `gcc` y `ninja` en el `PATH`.

Si SDL2 no está instalado, CMake intenta descargarlo y compilarlo durante la configuración.
