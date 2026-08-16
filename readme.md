# fractus-x64

Nuevo arbol de trabajo para el port moderno de Fractus.

## Estructura inicial

- `src/app`: arranque y composicion de la aplicacion.
- `src/core`: logica portable y algoritmos.
- `src/ui`: interfaz y comportamiento visual en coordenadas logicas.
- `src/platform`: integracion con la plataforma y futuras librerias base.
- `paletas`: ficheros de configuación de paletas de colores.
- `fractus.fon`: fuentes de letra usadas en la interfaz gráfica.
- `fractus.cfg`: configuración persistente.

El codigo original del proyecto permanece intacto fuera de esta carpeta.

## Build

Este arbol ya se considera el punto de partida del port moderno:

- estandar de lenguaje: **C11**
- sistema de build: **CMake**
- backend inicial de plataforma: **SDL2**

Ejemplo de uso:

```powershell
cmake -S . -B build
cmake --build build
```

Si SDL2 no esta instalado en el sistema, CMake intentara resolverlo automaticamente durante la configuracion.

La configuracion antigua basada en Borland/Turbo C queda solo como referencia historica en la raiz del proyecto original.
