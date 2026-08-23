# Guía de Diseño de UI para Diálogos de Fractus-x64

Esta guía establece las **reglas de diseño, arquitectura de coordenadas relativas $(x_0, y_0)$ y métricas oficiales** para todas las ventanas modales y diálogos de configuración, parámetros y gestión de ficheros en **Fractus-x64**. 

Cualquier diálogo nuevo que se añada a la aplicación en el futuro debe construirse siguiendo estrictamente las especificaciones descritas en este documento.

---

## 1. Principios Generales de Diseño

* **Resolución Lógica:** La interfaz de Fractus opera sobre un lienzo base de **$640 \times 480\text{ px}$** ($X: 0 \dots 639$, $Y: 0 \dots 479$).
* **Arquitectura de Coordenadas Relativas $(x_0, y_0)$:** 
  Cada diálogo define explícitamente sus coordenadas base `x0, y0` como la esquina superior izquierda de la ventana. **Absolutamente todas las coordenadas internas (ventanas, paneles, campos, etiquetas, botones, tips) se calculan sumando deltas a `x0` e `y0`** (`x0 + dx`, `y0 + dy`).
* **Centrado Horizontal:** Todo diálogo debe estar centrado simétricamente respecto al eje medio de la pantalla ($X = 320$):
  $$x_0 = \left\lfloor\frac{640 - W}{2}\right\rfloor, \quad x_1 = x_0 + W - 1$$
* **Centrado Vertical:** La coordenada superior $y_0$ de la ventana se calcula en base a la altura total $H$ del diálogo para que los márgenes superior e inferior sean idénticos:
  $$y_0 = \left\lfloor\frac{480 - H}{2}\right\rfloor, \quad y_1 = y_0 + H - 1$$
* **Fondo:** Todos los diálogos modales se renderizan superpuestos sobre el menú principal enmarcado (`fractus_app_render_main_menu(framebuffer, fonts, -1)`).

---

## 2. Anatomía y Dimensiones de la Ventana

### Anchuras Estándar

| Tipo de Diálogo | Ancho ($W$) | Origen $x_0$ | Rango Horizontal ($X_1 \dots X_2$) | Margen a Pantalla |
|---|---|---|---|---|
| **Diálogo Estándar (Parámetros / Config)** | **$370\text{ px}$** | `135` | `135 .. 504` | $135\text{ px}$ izq. / $135\text{ px}$ dcha. |
| **Atractores Dinámicos (Lorenz + 3D)** | **$410\text{ px}$** | `115` | `115 .. 524` | $115\text{ px}$ izq. / $115\text{ px}$ dcha. |
| **Diálogo de Selección / Modo de Vídeo** | **$320\text{ px}$** | `160` | `160 .. 479` | $160\text{ px}$ izq. / $160\text{ px}$ dcha. |
| **Selector de Archivos / Cargar** | **$300\text{ px}$** | `170` | `170 .. 469` | $170\text{ px}$ izq. / $170\text{ px}$ dcha. |
| **Diálogo de Error / Notificación Simple** | **$340\text{ px}$** | `150` | `150 .. 489` | $150\text{ px}$ izq. / $150\text{ px}$ dcha. |
| **Pantalla Completa Enmarcada (Paletas)** | **$568\text{ px}$** | `36` | `36 .. 603` | $36\text{ px}$ izq. / $36\text{ px}$ dcha. |

### Barra de Título
* La ventana dibuja su barra superior desde **$y_0 + 4$** hasta **$y_0 + 23$** (altura de $19\text{ px}$, fondo azul cian).
* El texto del título se dibuja centrado horizontalmente en **$x_0 + W/2$** a una altura de **$y = y_0 + 4$** utilizando la fuente `FRACTUS_FONT_ARIAL` en color blanco (`15u`).

---

## 3. Sistema de Separaciones Verticales (Reglas Métricas)

```
┌──────────────────────────────────────────────────────────────┐  y0
│ [================== TÍTULO DEL DIÁLOGO ==================]   │  y0 + 4 .. y0 + 23 (Barra de título)
│                                                              │
│   ┌── Panel 1 (Group Box) ───────────────────────────────┐   │  y_panel1_top = y0 + 32 (9 px bajo barra)
│   │                                                      │   │  y_field0 = y_panel1_top + 16 (9 px bajo título panel)
│   │  Etiqueta              [ Campo ]  [-]  [+]           │   │  y_label0 = y_field0 + 5
│   │  Etiqueta              [ Campo ]  [-]  [+]           │   │  Paso uniforme entre filas = 26 px
│   └──────────────────────────────────────────────────────┘   │  y_panel1_bottom = y_last_field_bottom + 10
│                                                              │
│                     (8 px exactos)                           │
│                                                              │
│   ┌── Panel 2 (Group Box) ───────────────────────────────┐   │  y_panel2_top = y_panel1_bottom + 8
│   │  (O) Opción 1                                        │   │  y_radio0 = y_panel2_top + 16
│   │  ( ) Opción 2                                        │   │
│   └──────────────────────────────────────────────────────┘   │  y_panel2_bottom = y_last_radio_bottom + 10
│                                                              │
│                     (8 px exactos)                           │
│                                                              │
│      Texto de ayuda / Tip explicativo con punto final.       │  y_tip = y_last_panel_bottom + 8
│                                                              │
│                     (18 px exactos)                          │
│                                                              │
│         [ Botón Acción ]         [ Botón Cancelar ]          │  y_btn = y_tip + 18 (Alto: 20 px)
│                                                              │
│                     (10 px exactos)                          │
└──────────────────────────────────────────────────────────────┘  y_bottom = y_btn + 20 + 10 = y_btn + 30
```

### Reglas Métricas Obligatorias:

1. **Borde superior de ventana $\to$ Panel superior (Group Box):**
   * Siempre **$y_0 + 32\text{ px}$** (margen limpio de **$9\text{ px}$** bajo la barra de título $y_0 + 23$).
2. **Panel superior $\to$ Primer campo / control:**
   * El borde superior del primer campo editable o control está siempre a **$16\text{ px}$** del borde superior del panel ($y_{\text{field0}} = y_{\text{panel\_top}} + 16$). Dado que el título del panel se renderiza centrado verticalmente sobre el borde superior ($y_{\text{panel\_top}} - 4 \dots y_{\text{panel\_top}} + 7$, altura 11 px), esto garantiza un espacio visual limpio de **$9\text{ px}$** entre el título del panel y el primer campo, eliminando cualquier superposición visual.
3. **Alineación vertical de Etiqueta $\to$ Campo numérico:**
   * La etiqueta de texto se dibuja a **$5\text{ px}$** por debajo del borde superior del campo numérico ($y_{\text{label}} = y_{\text{field}} + 5$), centrando verticalmente la tipografía de 11 px respecto al campo de 20 px.
4. **Paso entre filas:**
   * Estándar unificado en todos los diálogos de parámetros: **$26\text{ px}$** (campo de 20 px + separación inter-fila de 6 px).
5. **Último control $\to$ Borde inferior del panel:**
   * **$10\text{ px}$** ($y_{\text{panel\_bottom}} = y_{\text{last\_control\_bottom}} + 10$).
6. **Separación entre paneles consecutivos:**
   * Siempre **$8\text{ px}$ exactos** ($y_{\text{next\_panel\_top}} = y_{\text{prev\_panel\_bottom}} + 8$).
7. **Borde inferior del último panel $\to$ Tip explicativo:**
   * Siempre **$8\text{ px}$ exactos** ($y_{\text{tip}} = y_{\text{last\_panel\_bottom}} + 8$).
8. **Tip explicativo $\to$ Botones inferiores:**
   * Siempre **$18\text{ px}$ exactos** ($y_{\text{btn\_top}} = y_{\text{tip}} + 18$).
9. **Borde inferior de los botones $\to$ Borde inferior de la ventana:**
   * Siempre **$10\text{ px}$ exactos** ($y_{\text{window\_bottom}} = y_{\text{btn\_bottom}} + 10 = y_{\text{btn\_top}} + 30$).

---

## 4. Componentes y Controles

### Paneles (Group Boxes)
* **Bordes horizontales:** Margen exterior de **$5\text{ px}$** respecto a la ventana (`[x0 + 5, x0 + W - 6]`).
  * Para ventana de ancho 370 ($x_0 = 135$), el group box abarca `[140, 499]`.
* **Márgenes interiores:** Margen lateral de **$10\text{ px}$** desde el borde del panel (`x0 + 15` a la izquierda, y `x0 + W - 16` a la derecha).
* **Colores:** Marco en gris oscuro (`8u`), título del panel en negro (`0u`).

### Campos Numéricos y Botones de Paso `[-]`/`[+]`
* **Dimensiones estándar (para diálogos de ancho 370):**
  * Etiquetas de texto: `x0 + 15` ($150$).
  * Campos numéricos: `x0 + 189` ($324$), ancho $73\text{ px}$, alto **$20\text{ px}$**.
  * Botón `[-]`: `[x0 + 269, x0 + 309]` ($[404, 444]$), ancho $40\text{ px}$, alto **$20\text{ px}$**.
  * Botón `[+]`: `[x0 + 314, x0 + 354]` ($[449, 489]$), ancho $40\text{ px}$, alto **$20\text{ px}$**. Margen derecho al panel de 10 px (`364 - 354 = 10`).
* **Dimensiones para Atractores Dinámicos (Panel derecho $W=281$, ancho ventana 410, $x_0 = 115$):**
  * Panel derecho: `[x0 + 123, x0 + 404]`.
  * Etiquetas: `x0 + 133` (margen izq. 10 px).
  * Campos numéricos: `[x0 + 264, x0 + 324]`, ancho $60\text{ px}$.
  * Botón `[-]`: `[x0 + 329, x0 + 359]`, ancho $30\text{ px}$.
  * Botón `[+]`: `[x0 + 364, x0 + 394]`, ancho $30\text{ px}$ (margen dcho. 10 px al borde `404`).
* **Gestión de Foco:** Solo un campo editable puede tener foco a la vez. Al interactuar con otros controles, el campo activo consolida su valor y cierra la edición.

### Botones Inferiores de Acción
* **Par Estándar de Botones ("Guardar" / "Cancelar" o "Dibujar" / "Cancelar"):**
  * Dos botones de ancho $100\text{ px}$ con separación de $20\text{ px}$ (ancho total del bloque $220\text{ px}$), centrados horizontalmente en la ventana:
    * Margen izquierdo: $\Delta x = (W - 220)/2$.
    * Botón izquierdo: `[x0 + Δx, x0 + Δx + 100]`.
    * Botón derecho: `[x0 + Δx + 120, x0 + Δx + 220]`.
  * En ventanas de $W = 370$ ($x_0 = 135$): `[x0 + 75, x0 + 175]` y `[x0 + 195, x0 + 295]`.
  * En ventanas de $W = 410$ ($x_0 = 115$): `[x0 + 95, x0 + 195]` and `[x0 + 215, x0 + 315]`.
  * En ventanas de $W = 320$ ($x_0 = 160$): `[x0 + 50, x0 + 150]` and `[x0 + 170, x0 + 270]`.
* **Botón Único Centrado ("Aceptar" / "Volver"):**
  * Ancho $100\text{ px}$ centrado en la ventana: $\Delta x = (W - 100)/2$.
  * En ventana de $W = 370$ ($x_0 = 135$, Menús de método): `[x0 + 135, x0 + 235]`.
  * En ventana de $W = 340$ ($x_0 = 150$, Error): `[x0 + 120, x0 + 220]`.
  * En ventana de $W = 568$ ($x_0 = 36$, Paletas): `[x0 + 234, x0 + 334]`.

### Tipografía y Textos
* **Títulos de Diálogo:** `FRACTUS_FONT_ARIAL`, tamaño estándar, color blanco (`15u`), centrado en `x0 + W/2`, `y0 + 4`.
* **Etiquetas y Controles:** `FRACTUS_FONT_SMALL`, color negro (`0u`) o blanco (`15u`).
* **Tips Explicativos:** `FRACTUS_FONT_SMALL`, color negro (`0u`), centrados en $X = 320$, terminados obligatoriamente en **punto final (`.`)**.

---

## 5. Matriz de Diálogos Existentes (Referencia y Validación)

Todos los diálogos de la aplicación cumplen con la arquitectura relativa $(x_0, y_0)$ y la fórmula de consistencia:

| Diálogo | Archivo fuente | Origen $(x_0, y_0)$ | Ventana ($X_1, Y_1, X_2, Y_2$) | Dimensiones ($W \times H$) | Panel 1 ($Y$) | Panel 2 ($Y$) | Tip ($Y$) | Botones ($Y$) | Margen V (Top/Bottom) |
|---|---|---|---|---|---|---|---|---|---|
| **Mandelbrot** | `src/app/mandelbrot.c` | `(135, 70)` | `(135, 70, 504, 408)` | $370 \times 339\text{ px}$ | `y0+32 .. y0+72` | `y0+80 .. y0+282` | `y0+290` | `y0+308 .. y0+328` | $70\text{ px} / 71\text{ px}$ |
| **Julia** | `src/app/julia.c` | `(135, 44)` | `(135, 44, 504, 434)` | $370 \times 391\text{ px}$ | `y0+32 .. y0+72` | `y0+80 .. y0+334` | `y0+342` | `y0+360 .. y0+380` | $44\text{ px} / 45\text{ px}$ |
| **Biomorfos** | `src/app/biomorphs.c` | `(135, 30)` | `(135, 30, 504, 448)` | $370 \times 419\text{ px}$ | `y0+32 .. y0+362` | — (1 panel) | `y0+370` | `y0+388 .. y0+408` | $30\text{ px} / 31\text{ px}$ |
| **Plasmas** | `src/app/plasma.c` | `(135, 122)` | `(135, 122, 504, 356)` | $370 \times 235\text{ px}$ | `y0+32 .. y0+72` | `y0+80 .. y0+178` | `y0+186` | `y0+204 .. y0+224` | $122\text{ px} / 123\text{ px}$ |
| **Atractores Dinámicos** | `src/app/attractors.c` | `(115, 55)` | `(115, 55, 524, 425)` | $410 \times 371\text{ px}$ | `y0+32 .. y0+314` (izq) | `y0+32 .. y0+314` (dcha) | `y0+322` | `y0+340 .. y0+360` | $55\text{ px} / 55\text{ px}$ |
| **Parámetros por Defecto** | `src/app/config.c` | `(135, 66)` | `(135, 66, 504, 412)` | $370 \times 347\text{ px}$ | `y0+32 .. y0+104` | `y0+112 .. y0+210` / `P3: y0+218 .. y0+290` | `y0+298` | `y0+316 .. y0+336` | $66\text{ px} / 67\text{ px}$ |
| **Modo de Vídeo** | `src/app/config.c` | `(160, 113)` | `(160, 113, 479, 366)` | $320 \times 254\text{ px}$ | `y0+32 .. y0+197` | — (1 panel) | `y0+205` | `y0+223 .. y0+243` | $113\text{ px} / 113\text{ px}$ |
| **Selector de Archivos** | `src/app/files.c` | `(170, 76)` | `(170, 76, 469, 404)` | $300 \times 329\text{ px}$ | `y0+48 .. y0+288` | — (lista de slots) | — | `y0+298 .. y0+318` | $76\text{ px} / 76\text{ px}$ |
| **Pantalla de Paletas** | `src/app/files.c` | `(36, 48)` | `(36, 48, 603, 430)` | $568 \times 383\text{ px}$ | `y0+48 .. y0+348` | — (rejilla paletas) | `y0+335` | `y0+353 .. y0+372` | $48\text{ px} / 49\text{ px}$ |
| **Modificar Color Paleta** | `src/app/files.c` | `(135, 146)` | `(135, 146, 504, 332)` | $370 \times 187\text{ px}$ | `y0+32 .. y0+130` | — (1 panel) | `y0+138` | `y0+156 .. y0+176` | $146\text{ px} / 147\text{ px}$ |
| **Diálogo de Error** | `src/app/app.c` | `(150, 187)` | `(150, 187, 489, 291)` | $340 \times 105\text{ px}$ | — (sin panel) | — | — | `y0+74 .. y0+94` | $187\text{ px} / 188\text{ px}$ |

