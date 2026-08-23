#!/usr/bin/env python3
"""
Herramienta bmp2fon.py para Fractus-x64.
Usa los bitmaps de src/fonts/ (arial.bmp, france.bmp, small.bmp, courier.bmp)
como única fuente de la verdad para regenerar el binario fractus.fon.

Permite además verificar byte a byte la integridad contra el archivo existente.
"""

import os
import sys
import struct
import argparse

try:
    from PIL import Image
    HAS_PIL = True
except ImportError:
    HAS_PIL = False

def read_bmp_pure_python(bmp_path):
    """Lector de BMP de 24 bits nativo sin dependencias externas."""
    with open(bmp_path, "rb") as f:
        data = f.read()

    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError(f"Formato BMP inválido en {bmp_path}")

    offset = struct.unpack_from("<I", data, 10)[0]
    header_size, width, height, planes, bpp, compression = struct.unpack_from("<Iiihhi", data, 14)

    if bpp != 24 or compression != 0:
        raise ValueError(f"Solo se admiten BMPs de 24bpp sin compresión (bpp={bpp}, comp={compression})")

    is_top_down = (height < 0)
    height = abs(height)

    row_bytes = width * 3
    padding = (4 - (row_bytes % 4)) % 4
    stride = row_bytes + padding

    grid = [[(0, 0, 0) for _ in range(width)] for _ in range(height)]

    for y in range(height):
        src_y = y if is_top_down else (height - 1 - y)
        row_offset = offset + src_y * stride
        for x in range(width):
            px_off = row_offset + x * 3
            b, g, r = data[px_off], data[px_off + 1], data[px_off + 2]
            grid[y][x] = (r, g, b)

    return width, height, grid

def load_bmp_pixels(bmp_path):
    """Carga los píxeles RGB de un BMP usando PIL si está disponible, o el lector nativo."""
    if HAS_PIL:
        img = Image.open(bmp_path).convert("RGB")
        width, height = img.size
        pil_pixels = img.load()
        grid = [[pil_pixels[x, y] for x in range(width)] for y in range(height)]
        return width, height, grid
    else:
        return read_bmp_pure_python(bmp_path)

def extract_glyphs_from_bmp(bmp_path):
    """
    Detecta los rectángulos delimitadores (gris medio ~128) y extrae los
    glifos ordenados por filas y columnas.
    """
    width, height, grid = load_bmp_pixels(bmp_path)

    boxes = []
    def is_gray(color):
        r, g, b = color
        return (90 <= r <= 165) and (90 <= g <= 165) and (90 <= b <= 165)

    for y in range(height):
        for x in range(width):
            if is_gray(grid[y][x]):
                left_is_gray = (x > 0 and is_gray(grid[y][x - 1]))
                top_is_gray = (y > 0 and is_gray(grid[y - 1][x]))

                # Detectar esquina superior izquierda
                if not left_is_gray and not top_is_gray:
                    bx = x
                    while bx < width and is_gray(grid[y][bx]):
                        bx += 1
                    box_w = bx - x

                    by = y
                    while by < height and is_gray(grid[by][x]):
                        by += 1
                    box_h = by - y

                    if box_w >= 3 and box_h >= 3:
                        is_box = True
                        for cx in range(x, x + box_w):
                            if not is_gray(grid[y + box_h - 1][cx]):
                                is_box = False
                                break
                        for cy in range(y, y + box_h):
                            if not is_gray(grid[cy][x + box_w - 1]):
                                is_box = False
                                break
                        if is_box:
                            boxes.append((x, y, box_w - 2, box_h - 2))

    if not boxes:
        raise ValueError(f"No se detectaron rectángulos de glifos en {bmp_path}")

    font_h = boxes[0][3]
    row_height_approx = font_h + 4
    boxes.sort(key=lambda b: (b[1] // row_height_approx, b[0]))

    glyphs = []
    for x, y, gw, gh in boxes:
        glyph_grid = []
        for r in range(gh):
            row = []
            for c in range(gw):
                pr, pg, pb = grid[y + 1 + r][x + 1 + c]
                row.append(1 if (pr > 128 or pg > 128 or pb > 128) else 0)
            glyph_grid.append(row)
        glyphs.append({"width": gw, "grid": glyph_grid})

    return font_h, glyphs

def serialize_font_face(glyph_height, glyphs, ftype=0):
    """Empaqueta una familia tipográfica en el formato de slice binario MHIDS."""
    N = len(glyphs)
    table_size = N * 6
    data_start = 17 + table_size
    table_bytes = bytearray()
    bitmap_bytes = bytearray()
    curr_offset = data_start

    for g in glyphs:
        w = g["width"]
        grid = g["grid"]
        glyph_data = bytearray()
        for r in range(glyph_height):
            for c in range(w):
                glyph_data.append(1 if grid[r][c] != 0 else 0)
        table_bytes += struct.pack("<HI", w, curr_offset)
        bitmap_bytes += glyph_data
        curr_offset += len(glyph_data)

    magic = b"MHIDS Fuente\x1a"
    header = magic + struct.pack("<HH", ftype, glyph_height)
    return header + table_bytes + bitmap_bytes

def build_fon_from_bitmaps(fonts_dir):
    """Genera el archivo fractus.fon completo a partir de los 4 bitmaps."""
    font_names = ["arial", "france", "small", "courier"]
    serialized = {}

    for name in font_names:
        bmp_path = os.path.join(fonts_dir, f"{name}.bmp")
        if not os.path.exists(bmp_path):
            raise FileNotFoundError(f"Falta el bitmap de tipografía: {bmp_path}")

        h, glyphs = extract_glyphs_from_bmp(bmp_path)
        print(f"  - {name}: {len(glyphs)} glifos extraídos (alto={h}px)")
        serialized[name] = serialize_font_face(h, glyphs)

    arial_bytes = serialized["arial"]
    france_bytes = serialized["france"]
    small_bytes = serialized["small"]
    courier_bytes = serialized["courier"]

    off_arial = 0
    sz_arial = len(arial_bytes)

    off_france = sz_arial
    sz_france = len(france_bytes)

    off_small = off_france + sz_france
    sz_small = len(small_bytes)

    off_courier = off_small + sz_small
    sz_courier = len(courier_bytes)

    archive = arial_bytes + france_bytes + small_bytes + courier_bytes
    layout = {
        "france": (off_france, sz_france),
        "arial": (off_arial, sz_arial),
        "small": (off_small, sz_small),
        "courier": (off_courier, sz_courier),
    }

    return archive, layout

def main():
    parser = argparse.ArgumentParser(description="Regenera fractus.fon a partir de bitmaps fuente en src/fonts/")
    parser.add_argument("--fonts-dir", default="", help="Directorio de bitmaps (por defecto src/fonts)")
    parser.add_argument("--out-fon", default="", help="Archivo de salida (por defecto fractus.fon)")
    parser.add_argument("--check-against", default="", help="Compara byte a byte contra un archivo .fon de referencia")
    args = parser.parse_args()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, "..", ".."))

    fonts_dir = args.fonts_dir
    if not fonts_dir:
        candidates = [
            os.path.join(project_root, "src", "fonts"),
            os.path.join(os.getcwd(), "src", "fonts"),
            "src/fonts"
        ]
        for c in candidates:
            if os.path.exists(c):
                fonts_dir = c
                break

    if not fonts_dir or not os.path.exists(fonts_dir):
        print(f"Error: No se encontró el directorio de fuentes en {fonts_dir}")
        sys.exit(1)

    out_fon = args.out_fon
    if not out_fon:
        out_fon = os.path.join(project_root, "fractus.fon")

    print(f"Generando fractus.fon desde bitmaps en: {fonts_dir}")
    archive, layout = build_fon_from_bitmaps(fonts_dir)

    print(f"\nLayout generado (Total: {len(archive)} bytes):")
    print(f"  france:  offset={layout['france'][0]:5d}, size={layout['france'][1]:5d}")
    print(f"  arial:   offset={layout['arial'][0]:5d}, size={layout['arial'][1]:5d}")
    print(f"  small:   offset={layout['small'][0]:5d}, size={layout['small'][1]:5d}")
    print(f"  courier: offset={layout['courier'][0]:5d}, size={layout['courier'][1]:5d}")

    check_file = args.check_against
    if check_file and os.path.exists(check_file):
        with open(check_file, "rb") as f:
            ref_bytes = f.read()
        if archive == ref_bytes:
            print(f"\n>>> Comparación binaria: IDÉNTICO byte a byte contra {check_file} (0 diferencias) <<<")
        else:
            diffs = sum(1 for a, b in zip(archive, ref_bytes) if a != b)
            diffs += abs(len(archive) - len(ref_bytes))
            print(f"\n>>> ADVERTENCIA: Se encontraron {diffs} diferencias binarias contra {check_file} <<<")

    with open(out_fon, "wb") as f:
        f.write(archive)
    print(f"\nGuardado archivo regenerado en: {out_fon}")

if __name__ == "__main__":
    main()
