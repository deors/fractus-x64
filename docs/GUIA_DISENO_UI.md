# Guía de Diseño de UI para Diálogos de Fractus-x64

Esta guía establece las **reglas de diseño y métricas oficiales** para todas las ventanas modales y diálogos de configuración, parámetros y gestión de ficheros en **Fractus-x64**. 

Cualquier diálogo nuevo que se añada a la aplicación en el futuro debe construirse siguiendo estrictamente las especificaciones descritas en este documento.

---

## 1. Principios Generales de Diseño

* **Resolución Lógica:** La interfaz de Fractus opera sobre un lienzo base de **$640 \times 480\text{ px}$** ($X: 0 \dots 639$, $Y: 0 \dots 479$).
* **Centrado Horizontal:** Todo diálogo debe estar centrado simétricamente respecto al eje medio de la pantalla ($X = 320$).
* **Centrado Vertical:** La coordenada superior $Y_1$ de la ventana se calcula en base a la altura total $H$ del diálogo para que los márgenes superior e inferior sean idénticos:
  $$Y_1 = \left\lfloor\frac{480 - H}{2}\right\rfloor, \quad Y_2 = Y_1 + H - 1$$
* **Fondo:** Todos los diálogos modales se renderizan superpuestos sobre el menú principal enmarcado (`fractus_app_render_main_menu(framebuffer, fonts, -1)`).

---

## 2. Anatomía y Dimensiones de la Ventana

### Anchuras Estándar

| Tipo de Diálogo | Rango Horizontal ($X_1 \dots X_2$) | Ancho ($W$) | Margen a Pantalla |
|---|---|---|---|
| **Diálogo Estándar (Parámetros / Config)** | `135 .. 504` | **$370\text{ px}$** | $135\text{ px}$ izq. / $135\text{ px}$ dcha. |
| **Diálogo de Selección / Lista / Selector** | `165 .. 474` ó `170 .. 469` | **$300\text{ px} – 310\text{ px}$** | $165\text{ px} – 170\text{ px}$ |
| **Diálogo de Error / Notificación Simple** | `150 .. 489` | **$340\text{ px}$** | $150\text{ px}$ |
| **Pantalla Completa Enmarcada (Paletas)** | `36 .. 603` | **$568\text{ px}$** | $36\text{ px}$ |

### Barra de Título
* La barra gráfica de título (`fractus_ui_draw_window`) abarca desde **$Y_1 + 4$** hasta **$Y_1 + 23$** (altura de $19\text{ px}$, fondo azul cian).
* El texto del título se dibuja centrado horizontalmente en **$X = 320$** a una altura de **$Y = Y_1 + 4$** utilizando la fuente `FRACTUS_FONT_ARIAL` en color blanco (`15u`).

---

## 3. Sistema de Separaciones Verticales (Reglas Métricas)

```
┌──────────────────────────────────────────────────────────────┐  Y1
│ [================== TÍTULO DEL DIÁLOGO ==================]   │  Y1 + 4 .. Y1 + 23
│                                                              │
│   ┌── Panel 1 (Group Box) ───────────────────────────────┐   │  Y1 + 32  (32 px desde Y1 / 9 px bajo barra)
│   │  Radios de método / opciones                         │   │
│   └──────────────────────────────────────────────────────┘   │  Y_panel1_bottom
│                                                              │
│                     (8 px exactos)                           │
│                                                              │
│   ┌── Panel 2 (Group Box) ───────────────────────────────┐   │  Y_panel2_top = Y_panel1_bottom + 8
│   │  Etiqueta              [ Campo ]  [-]  [+]           │   │
│   │  Etiqueta              [ Campo ]  [-]  [+]           │   │
│   └──────────────────────────────────────────────────────┘   │  Y_panel2_bottom
│                                                              │
│                     (8 px exactos)                           │
│                                                              │
│      Texto de ayuda / Tip explicativo con punto final.       │  Y_tip = Y_panel_bottom + 8
│                                                              │
│                     (18 px exactos)                          │
│                                                              │
│         [ Botón Acción ]         [ Botón Cancelar ]          │  Y_btn = Y_tip + 18 (Alto: 20 px)
│                                                              │
│                     (10 px exactos)                          │
└──────────────────────────────────────────────────────────────┘  Y2 = Y_btn + 20 + 10 = Y_btn + 30
```

### Reglas Métricas Obligatorias:

1. **Borde superior de la ventana $\to$ Panel superior (Group Box):**
   * Siempre **$32\text{ px}$** desde $Y_1$ (equivalente a un margen limpio de **$9\text{ px}$** bajo la barra de título $Y_1 + 23$).
2. **Separación entre paneles consecutivos:**
   * Siempre **$8\text{ px}$ exactos** entre el borde inferior de un panel y el borde superior del siguiente ($Y_{\text{panel2\_top}} = Y_{\text{panel1\_bottom}} + 8$).
3. **Borde inferior del último panel $\to$ Tip explicativo:**
   * Siempre **$8\text{ px}$ exactos** ($Y_{\text{tip}} = Y_{\text{panel\_bottom}} + 8$).
4. **Tip explicativo $\to$ Botones inferiores:**
   * Siempre **$18\text{ px}$ exactos** ($Y_{\text{btn\_top}} = Y_{\text{tip}} + 18$).
5. **Borde inferior de los botones $\to$ Borde inferior de la ventana:**
   * Siempre **$10\text{ px}$ exactos** ($Y_2 = Y_{\text{btn\_bottom}} + 10$).

---

## 4. Componentes y Controles

### Paneles (Group Boxes)
* **Bordes horizontales:** Margen exterior de **$5\text{ px}$** respecto al borde de la ventana.
  * Para ventana `135 .. 504`, el group box abarca `140 .. 499`.
* **Márgenes interiores:** Margen de **$10\text{ px}$** desde el borde lateral del panel hasta los textos o componentes de los extremos.
* **Colores:** Marco en gris oscuro (`8u`), título de panel en negro (`0u`).

### Campos Numéricos y Botones de Paso `[-]`/`[+]`
* **Dimensiones:**
  * Campos numéricos: ancho $43\text{ px} – 73\text{ px}$, alto **$21\text{ px}$**.
  * Botones `[-]` y `[+]`: ancho **$41\text{ px}$**, alto **$21\text{ px}$**.
* **Separación entre filas de parámetros:** Paso vertical de **$26\text{ px} – 28\text{ px}$**.
* **Pasos estándar por parámetro:**
  * Iteraciones máximas: **$4$** (rango 16–1024).
  * Radio de escape al cuadrado: **$2$** (rango 4–1000).
  * Umbral de escape (Biomorfos): **$1$** (rango 1–100).
  * Canales de color VGA (Rojo, Verde, Azul): **$1$** (rango 0–63).
  * Número de círculos (Plasma): **$5$** (rango 10–500).
* **Gestión de Foco:** Solo un campo editable puede tener el foco a la vez. Al hacer clic en un control, el resto de campos consolida su valor y sale del modo de edición.

### Botones Inferiores de Acción
* **Par Estándar ("Guardar" / "Cancelar" ó "Dibujar" / "Cancelar"):**
  * Botón izquierdo (Acción/Aceptar): `FRACTUS_APP_RECT(210, Y_btn, 310, Y_btn + 20)` (ancho 101 px, alto 21 px).
  * Botón derecho (Cancelar): `FRACTUS_APP_RECT(330, Y_btn, 430, Y_btn + 20)` (ancho 101 px, alto 21 px).
  * Separación horizontal entre ambos botones: **$20\text{ px}$**.
* **Botón Único Centrado ("Volver" ó "Aceptar"):**
  * `FRACTUS_APP_RECT(270, Y_btn, 370, Y_btn + 20)` (ancho 101 px, alto 21 px, centrado en $X = 320$).

### Tipografía y Textos
* **Títulos de Diálogo:** `FRACTUS_FONT_ARIAL`, tamaño estándar, color blanco (`15u`).
* **Etiquetas y Controles:** `FRACTUS_FONT_SMALL`, color negro (`0u`) o blanco (`15u`).
* **Tips Explicativos:** `FRACTUS_FONT_SMALL`, color negro (`0u`), siempre centrados en $X = 320$ y terminados obligatoriamente en **punto final (`.`)**.

---

## 5. Matriz de Diálogos Existentes (Referencia y Validación)

Todos los diálogos actuales de la aplicación han sido ajustados y verificados bajo estas reglas:

| Diálogo | Archivo fuente | Ventana ($X_1, Y_1, X_2, Y_2$) | Dimensiones ($W \times H$) | Panel Top ($Y$) | Entre Paneles | Panel $\to$ Tip | Tip $\to$ Botones | Botones $\to$ Ventana | Centrado Vertical |
|---|---|---|---|---|---|---|---|---|---|
| **Mandelbrot** | `src/app/fractal.c` | `(135, 65, 504, 415)` | $370 \times 351\text{ px}$ | $Y = 97$ ($+32\text{ px}$) | **$8\text{ px}$** ($136 \to 144$) | **$8\text{ px}$** ($359 \to 367$) | **$18\text{ px}$** ($367 \to 385$) | **$10\text{ px}$** ($405 \to 415$) | $65\text{ px} / 64\text{ px}$ |
| **Julia** | `src/app/fractal.c` | `(135, 51, 504, 428)` | $370 \times 378\text{ px}$ | $Y = 83$ ($+32\text{ px}$) | **$8\text{ px}$** ($122 \to 130$) | **$8\text{ px}$** ($372 \to 380$) | **$18\text{ px}$** ($380 \to 398$) | **$10\text{ px}$** ($418 \to 428$) | $51\text{ px} / 51\text{ px}$ |
| **Biomorfos** | `src/app/fractal.c` | `(135, 57, 504, 423)` | $370 \times 367\text{ px}$ | $Y = 89$ ($+32\text{ px}$) | — (1 panel) | **$8\text{ px}$** ($367 \to 375$) | **$18\text{ px}$** ($375 \to 393$) | **$10\text{ px}$** ($413 \to 423$) | $57\text{ px} / 57\text{ px}$ |
| **Plasma** | `src/app/fractal.c` | `(160, 140, 479, 341)` | $320 \times 202\text{ px}$ | $Y = 172$ ($+32\text{ px}$) | — (1 panel) | **$8\text{ px}$** ($285 \to 293$) | **$18\text{ px}$** ($293 \to 311$) | **$10\text{ px}$** ($331 \to 341$) | $140\text{ px} / 139\text{ px}$ |
| **Parámetros por Defecto** | `src/app/config.c` | `(135, 123, 504, 355)` | $370 \times 233\text{ px}$ | $Y = 155$ ($+32\text{ px}$) | **$8\text{ px}$** ($223 \to 231$) | **$8\text{ px}$** ($299 \to 307$) | **$18\text{ px}$** ($307 \to 325$) | **$10\text{ px}$** ($345 \to 355$) | $123\text{ px} / 124\text{ px}$ |
| **Modo de Vídeo** | `src/app/config.c` | `(165, 107, 474, 371)` | $310 \times 265\text{ px}$ | $Y = 139$ ($+32\text{ px}$) | — (1 panel) | **$8\text{ px}$** ($315 \to 323$) | **$18\text{ px}$** ($323 \to 341$) | **$10\text{ px}$** ($361 \to 371$) | $107\text{ px} / 108\text{ px}$ |
| **Cargar Dibujo / Paleta** | `src/app/files.c` | `(170, 76, 469, 404)` | $300 \times 329\text{ px}$ | $Y = 108$ ($+32\text{ px}$) | — (lista de 10 slots) | — (sin tip) | **$18\text{ px}$** ($356 \to 374$) | **$10\text{ px}$** ($394 \to 404$) | $76\text{ px} / 76\text{ px}$ |
| **Pantalla de Paletas** | `src/app/files.c` | `(36, 48, 603, 430)` | $568 \times 383\text{ px}$ | $Y = 80$ ($+32\text{ px}$) | — (1 rejilla) | **$8\text{ px}$** ($375 \to 383$) | **$18\text{ px}$** ($383 \to 401$) | **$10\text{ px}$** ($420 \to 430$) | $48\text{ px} / 49\text{ px}$ |
| **Modificar Color** | `src/app/files.c` | `(135, 142, 504, 336)` | $370 \times 195\text{ px}$ | $Y = 174$ ($+32\text{ px}$) | — (1 panel) | **$8\text{ px}$** ($280 \to 288$) | **$18\text{ px}$** ($288 \to 306$) | **$10\text{ px}$** ($326 \to 336$) | $142\text{ px} / 143\text{ px}$ |
| **Diálogo de Error** | `src/app/app.c` | `(150, 184, 489, 296)` | $340 \times 113\text{ px}$ | $Y = 216$ ($+32\text{ px}$) | — (sin panel) | — (mensaje $Y=230$) | **$18\text{ px}$** ($240 \to 258$) | **$10\text{ px}$** ($278 \to 288$) | $184\text{ px} / 184\text{ px}$ |
