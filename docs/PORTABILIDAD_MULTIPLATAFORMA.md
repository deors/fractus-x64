# Análisis de portabilidad multiplataforma de **Fractus**

## Objetivo

Definir las decisiones de arquitectura que conviene tomar **ahora** para que la adaptación de **Fractus** no quede limitada a Windows x64, sino que sea factible mantenerla en una sola base de código para **Windows, Linux y macOS**.

La idea no es “portar a Windows y luego ya veremos”, sino diseñar desde el principio un núcleo portátil con una capa de plataforma pequeña y sustituible.

## Conclusión ejecutiva

Sí es factible hacer **Fractus** multiplataforma, pero sólo si se toman desde el principio estas decisiones:

1. **Separar núcleo, frontend y plataforma**.
2. **Elegir librerías multiplataforma como dependencia base**, evitando APIs nativas directas salvo detrás de una abstracción.
3. **Congelar formatos, tipos y contratos internos** para que no dependan de compilador, endianess, sistema de archivos o backend gráfico.
4. **Modelar la aplicación como framebuffer + eventos**, no como acceso a hardware o polling BIOS.
5. **Adoptar un sistema de build y empaquetado multiplataforma** desde el día 1.

Si no se hace así, el riesgo es acabar con un “Fractus para Windows” que use decisiones irreversibles y obligue a un segundo port para Linux/macOS.

## Decisiones de arquitectura recomendadas

## 1. Separar el proyecto en capas claras

La decisión más importante es dejar de pensar el programa como un único bloque de C y dividirlo en módulos con responsabilidades nítidas.

### Propuesta de capas

| Capa | Responsabilidad | Debe conocer |
| --- | --- | --- |
| `core` | algoritmos fractales, paletas, formatos, estado de aplicación | tipos portables y servicios abstractos |
| `ui` | menús, ventanas, interacción lógica, layout en coordenadas lógicas | framebuffer, texto, input abstracto |
| `platform` | ventana, eventos, reloj, filesystem, audio si llega a existir | SDL2/SDL3 u otra API concreta |
| `app` | arranque, composición de módulos, ciclo principal | `core`, `ui`, `platform` |

### Decisión concreta

Hay que evitar que `fractal.c`, `programa.c` o la futura lógica de UI conozcan:

- SDL directamente,
- Win32,
- POSIX,
- Cocoa,
- rutas nativas,
- y detalles del backend gráfico.

Eso debe vivir sólo en `platform`.

## 2. Mantener un **canvas lógico** fijo

La UI actual asume resoluciones concretas y coordenadas absolutas. Si se intenta rediseñarla en términos de ventanas modernas y layouts responsivos desde el principio, el riesgo de rehacer demasiado es alto.

### Decisión recomendada

Mantener un **espacio lógico interno**:

- principal: `640x480`,
- visor de paleta: `320x200`,
- y otros modos heredados sólo como tamaños lógicos de render.

La ventana real en Windows/Linux/macOS sólo presenta ese canvas:

- con escalado entero cuando sea posible,
- con letterboxing cuando haga falta,
- y con remapeo de ratón a coordenadas lógicas.

### Por qué es importante

Esto desacopla:

- la UI histórica,
- la resolución física,
- el DPI,
- el backend de render,
- y las diferencias entre monitores y sistemas operativos.

## 3. Usar un framebuffer propio como contrato central

El contrato interno no debe ser “dibujar con la API X”, sino “modificar un framebuffer lógico del programa”.

### Decisión recomendada

Definir una estructura central similar a:

- ancho y alto lógicos,
- buffer de índices de color,
- paleta de 256 entradas,
- primitivas de lectura/escritura de píxel,
- blit de regiones,
- presentación a pantalla como operación separada.

### Consecuencia arquitectónica

Los algoritmos fractales y buena parte de la UI deberían trabajar sobre ese framebuffer, no sobre SDL, OpenGL, Metal, Direct2D ni APIs nativas.

Eso hace que:

- el núcleo siga siendo portable,
- los tests sean más fáciles,
- y el backend gráfico sea intercambiable.

## 4. Preservar la paleta indexada como modelo interno

El programa original está diseñado alrededor de una paleta indexada de 256 colores y muchos comportamientos dependen de ello.

### Decisión recomendada

Internamente, **Fractus** debe seguir funcionando en términos de:

- índice de color,
- paleta editable,
- fundidos por paleta,
- imágenes guardadas con índices.

La conversión a RGBA debe ocurrir **solo** en la capa de presentación.

### Qué evita esta decisión

- perder fidelidad respecto al original,
- complicar los formatos existentes,
- y mezclar la lógica artística del programa con la tecnología del backend.

## 5. Adoptar un modelo de eventos, no de polling BIOS

El código actual usa espera bloqueante y polling de teclado/ratón. En multiplataforma eso sólo es sostenible si se reconstruye sobre una cola de eventos moderna.

### Decisión recomendada

Definir una capa de input abstracto con eventos del tipo:

- mouse move,
- mouse down,
- mouse up,
- key down,
- key up,
- quit,
- resize,
- timer tick.

La UI podrá seguir exponiendo helpers de estilo clásico (`Menu`, `EsperaPulsar`, etc.), pero implementados sobre eventos modernos.

### Recomendación práctica

No dejar que la lógica de aplicación lea SDL directamente. Mejor:

- `platform_poll_events(...)`
- traduce a eventos propios,
- `app_dispatch_event(...)`
- y la UI decide.

## 6. Elegir una dependencia base realmente multiplataforma

### Recomendación principal

Usar **SDL2** o **SDL3** como base para:

- ventana,
- render/presentación,
- teclado,
- ratón,
- temporización,
- rutas de preferencias si se quiere,
- y soporte básico cross-platform.

### Decisión estratégica

Aunque se usen diálogos nativos, hilos o filesystem adicional, la aplicación no debería depender de Win32, X11, Wayland o Cocoa desde el código de dominio.

### SDL2 vs SDL3

- **SDL2**: más madura, documentación y ejemplos abundantes, menor riesgo inicial.
- **SDL3**: más moderna, pero para un rescate de código antiguo puede añadir fricción innecesaria.

**Recomendación:** empezar con **SDL2** salvo que haya un motivo fuerte para SDL3.

## 7. Sistema de build único y portátil

### Decisión obligatoria

Adoptar **CMake** como sistema de build principal.

### Por qué

Permite generar:

- Visual Studio en Windows,
- Ninja/Makefiles en Linux,
- Xcode o Ninja en macOS.

Además facilita:

- integración de SDL,
- builds Debug/Release,
- CI multiplataforma,
- empaquetado posterior.

### Qué no conviene hacer

- mantener proyectos IDE específicos como fuente principal,
- scripts de compilación distintos por plataforma,
- ni macros de compilador Borland heredadas.

## 8. Tipos fijos y serialización explícita

Los formatos actuales escriben enteros y arrays directamente a disco. Eso es una fuente de bugs silenciosos en portabilidad.

### Decisión recomendada

Usar tipos explícitos:

- `uint8_t`,
- `int32_t`,
- `uint32_t`,
- `size_t` sólo para memoria, no para disco.

Y definir serialización explícita para:

- `.MHG`,
- `.DRSP`,
- `.MHF`,
- configuración.

### Regla clave

Los formatos en disco no deben depender de:

- tamaño de `int`,
- alineación del compilador,
- packing implícito,
- ni representación interna de structs.

## 9. Rutas, recursos y directorios sin supuestos DOS

Aquí hay que tomar una decisión importante pronto, porque afecta a guardado, carga, instalación y empaquetado.

### Recomendación

Separar claramente:

| Tipo de recurso | Estrategia |
| --- | --- |
| recursos del programa (`.fon`, paletas por defecto, etc.) | carpeta de recursos de la app o assets empaquetados |
| configuración del usuario | directorio de preferencias por usuario |
| dibujos/paletas del usuario | diálogos de abrir/guardar o rutas elegidas por el usuario |

### Decisiones concretas

1. No asumir letras de unidad.
2. No asumir separador `\`.
3. No asumir filesystem case-insensitive.
4. No asumir que el directorio de trabajo es el de la app.
5. Resolver rutas de recursos desde una capa de runtime.

### Especialmente en macOS

macOS empaqueta apps como bundles; si el programa depende de “estar lanzado desde su carpeta”, luego habrá problemas.

## 10. UTF-8 interno y tratamiento explícito de textos heredados

El código y textos vienen de una era pre-Unicode. En multiplataforma esto acaba dando problemas rápido.

### Decisión recomendada

- Usar **UTF-8** como codificación interna del proyecto.
- Decidir explícitamente qué hacer con textos históricos y caracteres extendidos.
- No depender de páginas de códigos locales del sistema.

### Implicaciones

- nombres de fichero,
- textos de UI,
- carga de recursos,
- y compilación en sistemas con locales distintas.

## 11. Desacoplar render de texto del backend

Las fuentes actuales `.MHF` son un activo valioso, pero la forma en que se pintan no debería depender del backend.

### Decisión recomendada

Mantener dos niveles:

1. **motor de fuentes raster propias** para compatibilidad estética;
2. backend gráfico que solo sabe poner píxeles o blitear un bitmap.

Eso permite:

- preservar el aspecto original,
- testear el texto sin ventana,
- y no acoplar la tipografía a SDL_ttf o APIs del sistema.

## 12. Definir un `Platform API` pequeño y estable

Cuanto más pequeña sea la API de plataforma, más fácil será mantener Windows/Linux/macOS.

### Propuesta de responsabilidades mínimas

- iniciar y cerrar aplicación,
- crear ventana,
- presentar framebuffer,
- obtener eventos,
- temporización,
- filesystem básico,
- abrir diálogo de fichero si se usa,
- logs y errores.

### Lo que no debe entrar

- lógica fractal,
- reglas de menús,
- layout,
- formatos,
- paletas,
- textos de negocio.

## 13. No mezclar lógica de aplicación con diálogos nativos

En Windows, Linux y macOS los diálogos de abrir/guardar se resuelven distinto. Si se embeben por todas partes en la lógica, luego cuesta cambiar de backend.

### Decisión recomendada

Definir operaciones abstractas:

- `request_open_file(...)`
- `request_save_file(...)`
- `request_choose_directory(...)`

y que la lógica de aplicación reciba sólo el resultado.

### Ventaja

Si un día se quiere:

- diálogo nativo,
- diálogo SDL,
- explorador propio clásico,

el resto del programa no cambia.

## 14. Diseñar el ciclo principal para seguir siendo monohilo

No parece necesario introducir concurrencia desde el principio.

### Decisión recomendada

Mantener:

- un hilo principal para UI y render,
- y cálculo síncrono inicialmente.

Si después se quiere cancelar render o calcular fractales en background, se puede añadir sobre una arquitectura limpia.

### Por qué conviene

- simplifica portabilidad,
- evita diferencias entre event loops,
- y reduce errores sutiles de sincronización.

## 15. Pensar en empaquetado desde el principio

No es solo un problema de compilar: en macOS y Windows la distribución cambia cómo se resuelven recursos y dependencias.

### Decisión recomendada

Diseñar el runtime para soportar:

- ejecutable + carpeta de assets,
- o app bundle con recursos internos,
- sin depender del directorio actual.

### Consecuencia

Los recursos deberían cargarse a través de una función común tipo:

- `app_get_resource_path("paletas/defecto.drsp")`

en lugar de abrir rutas relativas directamente desde todo el código.

## 16. Introducir tests de núcleo y validaciones de formato

Para que la multiplataforma sea sostenible, hace falta verificar el núcleo sin necesidad de abrir una ventana.

### Decisión recomendada

Añadir tests al menos para:

- serialización/deserialización de `.MHG`, `.DRSP`, `.MHF`,
- generación de paletas,
- algoritmos fractales sobre framebuffers de prueba,
- remapeo de coordenadas lógicas.

No hace falta empezar por tests gráficos complejos, pero sí por los contratos portables.

## 17. Preparar CI multiplataforma desde el inicio

### Recomendación

Montar CI en:

- Windows,
- Linux,
- macOS.

Aunque al principio sólo compile y ejecute tests unitarios básicos.

### Qué evita

- introducir dependencias accidentales de una plataforma,
- romper macOS o Linux sin enterarse,
- y aceptar supuestos del compilador local.

## 18. Evitar decisiones que cierren el futuro

Estas son decisiones que **no** conviene tomar si el objetivo es multiplataforma:

- usar Win32 como API principal del programa;
- modelar archivos y rutas con semántica Windows;
- usar OpenGL/Metal/Vulkan directamente en el núcleo;
- meter filesystem, input y render dentro de `fractal.c` o de la lógica de UI;
- mantener escritura binaria “tal cual en memoria”;
- depender del current working directory;
- introducir condicionantes de un único IDE.

## Arquitectura objetivo sugerida

### Árbol orientativo

```text
src/
  app/
    main.c
    app.c
    app.h
  core/
    fractal.c
    fractal.h
    palette.c
    palette.h
    image.c
    image.h
    formats.c
    formats.h
  ui/
    menus.c
    menus.h
    widgets.c
    widgets.h
    fonts.c
    fonts.h
  platform/
    platform.h
    sdl_platform.c
    sdl_files.c
    sdl_time.c
  assets/
    ...
```

## Orden recomendado de decisiones

### Primero

1. **C11/CMake** como base del proyecto.
2. **SDL2** como dependencia multiplataforma principal.
3. **framebuffer indexado + paleta** como contrato central.
4. **platform API** pequeño.
5. **formatos con tipos fijos**.

### Después

1. separar `libreria.c` por responsabilidades;
2. portar entrada y render;
3. portar formatos;
4. portar UI;
5. pulir empaquetado y distribución.

## Recomendación final

La decisión correcta no es “hacerlo portable” como una capa final, sino **convertir la portabilidad en una propiedad de la arquitectura**.

Si tuviera que resumirlo en una sola frase:

> **Fractus debe reescribirse como un núcleo C portátil que renderiza a un framebuffer indexado y delega ventana, eventos, reloj y filesystem en una capa de plataforma mínima, idealmente sobre SDL2 y CMake.**

Con esa base, Windows, Linux y macOS pasan a ser un problema de integración y empaquetado, no tres ports distintos.
