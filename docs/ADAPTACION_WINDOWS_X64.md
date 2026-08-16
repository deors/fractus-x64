# Análisis de viabilidad para adaptar **Fractus** a **Windows x64**

## Resumen ejecutivo

La adaptación es **viable**, pero **no** como recompilación directa del código actual. El proyecto está escrito para **MS-DOS 16 bits** y depende de:

- extensiones de compilador de Borland/Turbo C,
- bibliotecas gráficas **BGI**,
- acceso directo a hardware VGA,
- interrupciones BIOS/DOS para ratón y teclado,
- y APIs de sistema de ficheros específicas de DOS.

En Windows x64 actual no existe compatibilidad nativa con ese modelo de ejecución, así que el camino realista es un **port**: conservar la lógica fractal y reimplementar la capa de plataforma (vídeo, entrada, temporización, directorios y persistencia binaria donde convenga).

Lo positivo es que buena parte de esa dependencia está concentrada en `libreria.c`, mientras que la lógica de generación de fractales en `fractal.c` es bastante reutilizable.

## Estado actual del proyecto

### Toolchain y organización

- El proyecto viene de entorno Borland/Turbo C; `fractus.prj` apunta a rutas tipo `C:\PROGS\BC\...`.
- Hay drivers BGI enlazados externamente: `EGAVGAF.OBJ`, `SVGA256.OBJ`, `VGA256.OBJ`.
- La estructura es poco moderna pero favorable al port:
  - `programa.c`: flujo principal y UI.
  - `fractal.c`: algoritmos fractales.
  - `libreria.c`: vídeo, ratón, teclado, directorio, fuentes, dibujo, formatos.
  - `incluye.c`: cabeceras, macros, tipos y prototipos.

## Hallazgos por subsistema

### 1. Vídeo y dibujo

La capa gráfica depende fuertemente de **BGI** y de VGA real:

- Inicialización de drivers BGI y modos de vídeo en `libreria.c:907-963`.
- Primitivas de dibujo basadas en `putpixel`, `getpixel`, `line`, `getimage`, `putimage`, `imagesize`, `floodfill`.
- Programación directa de la paleta VGA por puertos:
  - `outportb(0x3c8/0x3c9)` y `inportb(0x3c7/0x3c9)` en `libreria.c:1021-1037`.

**Conclusión:** esta parte no es portable tal cual. En x64 hay que sustituirla completa por una capa moderna.

**Impacto funcional:**

- modos 320x200, 640x350, 640x400, 640x480, 800x600, 1024x768, 1280x1024;
- paleta indexada de 256 colores;
- efectos de fundido por paleta;
- lectura/escritura de píxel por índice de color;
- copia de regiones de pantalla para ventanas y botones.

### 2. Ratón, teclado e interrupciones

El proyecto usa BIOS/driver de ratón DOS:

- `int 33h` y `int86(0x33, ...)` en `libreria.c:1133-1219`.
- ensamblador inline `asm mov ... / int 33h`.
- teclado por `bioskey()` y `getch()` en múltiples puntos (`libreria.c:364+`, `1495+`).

**Conclusión:** hay que reemplazarlo por un modelo de eventos de Windows o una librería multiplataforma.

**Observación importante:** la semántica actual está muy orientada a polling bloqueante (`EsperaPulsar`, `EsperaSoltar`, `Menu`). Eso se puede conservar con wrappers, pero internamente conviene migrarlo a cola de eventos.

### 3. Modos de vídeo y coordenadas de UI

La UI está dibujada con coordenadas absolutas y asume resoluciones concretas:

- menús con rectángulos fijos en `programa.c`;
- pantalla principal en 640x480;
- pantalla de paleta en 320x200;
- restricciones de ratón usando coordenadas de esos modos.

**Conclusión:** el port es viable si se mantiene un **canvas lógico fijo** y se escala a la ventana real. Eso evita reescribir toda la UI.

La estrategia más segura es:

1. mantener internamente superficies lógicas 640x480 y 320x200;
2. renderizarlas en una ventana reescalada;
3. remapear coordenadas de ratón desde ventana real a coordenada lógica.

### 4. Sistema de ficheros y DOS

Hay dependencia explícita de APIs DOS:

- `getdisk`, `setdisk`, `findfirst`, `findnext`, `ffblk`, `chdir` en `libreria.c:504-791`;
- listado temporal a fichero `temp`;
- lógica de selección de unidad `A:`-`Z:`.

**Conclusión:** esta parte también requiere sustitución.

En Windows x64 moderno:

- la noción de “unidad disponible” puede existir, pero no debe resolverse como en DOS;
- `findfirst/findnext` y `ffblk` no existen en ese modelo;
- la UI de exploración propia sería más fácil de reimplementar usando APIs modernas o, mejor, con diálogo nativo de abrir/guardar.

### 5. Extensiones de memoria y compilador

Hay varios elementos no compatibles con compiladores x64 actuales:

- `far`, `huge`, `_cdecl`;
- `coreleft()` y `farcoreleft()`;
- ensamblador inline estilo Borland;
- inclusión de `.c` como si fuesen cabeceras (`#include <incluye.c>`, `#include <fractal.c>`).

**Conclusión:** el código necesita una limpieza mínima de compilación antes de poder llevarse a MSVC, Clang o MinGW-w64.

### 6. Persistencia y formatos propios

Hay formatos binarios propios:

- gráficos `.MHG` en `LeeGrafico/HazGrafico` (`libreria.c:273-360`);
- paletas `.MHP` (`libreria.c:1059-1085`);
- fuentes `.MHF` (`libreria.c:823-904`);
- configuración binaria `fractus.cfg` (`programa.c:740-769`).

**Lo bueno:** los formatos están descritos implícitamente en el código y se pueden seguir soportando.

**Riesgos:**

- se escriben arrays y enteros binarios “en bruto”;
- la compatibilidad depende del tamaño de tipos y del orden de bytes;
- aunque en Windows x64 little-endian `int` seguirá siendo de 32 bits en compiladores comunes, es mejor no depender de eso.

**Recomendación:** definir estructuras de disco con tipos fijos (`uint8_t`, `uint32_t`, etc.) y rutinas explícitas de serialización.

### 7. Núcleo fractal reutilizable

La parte algorítmica en `fractal.c` es la más portable:

- usa `resx`, `resy`, `ncol`;
- llama a primitivas abstractas como `PonPunto`, `LeePunto`, `EstadoRaton`.

**Conclusión:** la lógica de fractales se puede conservar casi intacta si se mantiene la misma API de backend o una equivalente.

## Viabilidad real

### Viable si el objetivo es:

- ejecutar **Fractus** de forma nativa en Windows x64;
- conservar la estética, menús, paleta indexada y formatos propios;
- aceptar reescritura de la capa DOS/BGI.

### No viable si el objetivo es:

- recompilar “casi sin tocar”;
- seguir usando interrupciones BIOS, puertos VGA o BGI original;
- ejecutar el `.EXE` actual de 16 bits directamente en Windows x64.

## Recomendación técnica

La opción más razonable es portar el proyecto a **C99/C11 + SDL2**.

### Por qué SDL2 encaja bien

- ventana, teclado, ratón y temporización en Windows x64;
- renderizado sencillo;
- fácil implementar un framebuffer indexado de 8 bits;
- se puede mantener la lógica de paleta y convertirla a textura RGB al presentar;
- permite escalar un canvas lógico sin rehacer la UI.

También sería posible usar Win32 puro, pero costaría más y daría menos flexibilidad.

## Qué habría que adaptar

| Área | Estado | Acción recomendada |
| --- | --- | --- |
| Algoritmos fractales | Bastante portable | Mantener y recompilar con cambios mínimos |
| BGI / drivers `.OBJ` | No portable | Sustituir por backend SDL2 |
| Paleta VGA por puertos | No portable | Mantener paleta en memoria y aplicarla al presentar |
| Ratón `int 33h` | No portable | Sustituir por eventos SDL2/Win32 |
| Teclado BIOS | No portable | Sustituir por eventos modernos |
| Explorador DOS | No portable | Rehacer con filesystem moderno o diálogo nativo |
| Formatos `.MHG/.MHP/.MHF/.CFG` | Recuperables | Mantener con serialización explícita |
| Menús y ventanas | Portables con esfuerzo | Reutilizar lógica, mantener coordenadas lógicas |

## Plan recomendado de adaptación

### Fase 1. Separar plataforma de lógica

1. Convertir `incluye.c` en un `.h` real y dejar de incluir `.c` desde otros `.c`.
2. Aislar una API de plataforma con operaciones equivalentes a:
   - inicializar vídeo,
   - cambiar modo lógico,
   - poner/leer píxel,
   - línea, rectángulo, blit,
   - paleta,
   - ratón,
   - teclado,
   - pausa/temporización.
3. Mantener `fractal.c` casi sin cambios.

### Fase 2. Nuevo backend Windows x64

1. Crear un backend SDL2:
   - ventana;
   - framebuffer lógico indexado;
   - tabla de paleta de 256 colores;
   - conversión a textura de presentación;
   - escalado de resolución.
2. Emular `PonVideoVGA()` y `PonVideoSVGA()` como cambio de tamaño lógico, no de modo hardware real.

### Fase 3. Entrada y UI

1. Sustituir `EstadoRaton`, `EsperaPulsar`, `EsperaSoltar`, `Menu`.
2. Remapear coordenadas del ratón a espacio lógico.
3. Rehacer `Memoriza/Recupera` sobre buffers de memoria del nuevo framebuffer.

### Fase 4. Ficheros y configuración

1. Rehacer `Directorio()` con APIs modernas:
   - opción A: diálogo nativo de abrir/guardar;
   - opción B: explorador propio sobre filesystem moderno.
2. Reescribir lectura/escritura de `.MHG`, `.MHP`, `.MHF` y `fractus.cfg` con tipos fijos.
3. Mantener compatibilidad con los ficheros existentes siempre que sea posible.

### Fase 5. Limpieza de compatibilidad

1. Eliminar `far`, `huge`, `_cdecl`, `asm`, `bios.h`, `graphics.h`, `dos.h`, `dir.h`.
2. Sustituir `randomize()` por `srand(time(NULL))` donde proceda.
3. Sustituir utilidades antiguas de Borland por equivalentes estándar.

### Fase 6. Validación funcional

1. Confirmar que la pantalla principal conserva el aspecto original.
2. Confirmar dibujo correcto en:
   - Mandelbrot,
   - plasma rectangular,
   - plasma circular.
3. Confirmar carga/grabación de paletas y dibujos.
4. Confirmar fuentes `.MHF` y textos.

## Estrategia de migración más segura

La ruta menos arriesgada es **no tocar primero la lógica fractal**, sino:

1. montar un esqueleto Windows x64 con backend nuevo;
2. implementar la API mínima que hoy expone `libreria.c`;
3. conectar después `fractal.c` y, por último, la UI y los diálogos.

Así el trabajo se divide en dos bloques:

- **núcleo reutilizable**, y
- **capa DOS a sustituir**.

## Riesgos principales

1. **Compatibilidad binaria de formatos**: por escritura directa de enteros y structs.
2. **UI rígida por coordenadas fijas**: mitigable con canvas lógico.
3. **Dependencia fuerte del comportamiento de paleta indexada**: mitigable con framebuffer 8-bit emulado.
4. **Acoplamiento de `libreria.c`**: aunque concentra mucho, mezcla plataforma, UI, ficheros y formatos; conviene dividirla.

## Conclusión

La adaptación a Windows x64 es **factible y razonable**, pero requiere una **reescritura de la capa de plataforma** y una **modernización mínima del código fuente**. No parece un proyecto perdido: el núcleo fractal y buena parte de la lógica de aplicación son recuperables.

Si se aborda como un port ordenado, la arquitectura actual permite conservar:

- algoritmos,
- interfaz clásica,
- paletas,
- fuentes,
- y formatos propios,

mientras se reemplazan por completo las piezas dependientes de MS-DOS/16 bits.
