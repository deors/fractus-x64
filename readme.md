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

Este árbol ya se considera el punto de partida del port moderno:

- estandar de lenguaje: **C11**
- sistema de build: **CMake**
- backend inicial de plataforma: **SDL2**

Ejemplo de uso:

```powershell
cmake -S . -B build
cmake --build build
```

Si SDL2 no está instalado en el sistema, CMake intentará resolverlo automáticamente durante la configuración.
