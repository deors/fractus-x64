#!/usr/bin/env python3
"""
Herramienta de extracción de tipografías desde fractus.fon a bitmaps BMP.
Genera un bitmap para cada familia tipográfica (arial, france, small, courier)
con todos los glifos ordenados y delimitados por un rectángulo en gris medio.
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

def write_bmp_24bpp(filename, width, height, rgb_grid):
    """Escribe un archivo BMP de 24bpp sin comprimir usando struct estándar."""
    # Cada fila en BMP debe tener un tamaño múltiplo de 4 bytes
    row_bytes = width * 3
    padding = (4 - (row_bytes % 4)) % 4
    image_size = (row_bytes + padding) * height
    file_size = 54 + image_size

    # BMP Header (14 bytes)
    header = struct.pack(
        "<2sIHHI",
        b"BM",
        file_size,
        0, 0,
        54
    )

    # DIB Header (BITMAPINFOHEADER - 40 bytes)
    dib = struct.pack(
        "<IIIHHIIIIII",
        40,
        width,
        height,  # positivo = de abajo a arriba
        1,       # planos
        24,      # bpp
        0,       # BI_RGB (sin compresión)
        image_size,
        2835,    # 72 DPI horizontal
        2835,    # 72 DPI vertical
        0,
        0
    )

    with open(filename, "wb") as f:
        f.write(header)
        f.write(dib)
        # Las filas en BMP estándar se escriben de abajo a arriba (y = height-1 hacia 0)
        for y in range(height - 1, -1, -1):
            row_data = bytearray()
            for x in range(width):
                r, g, b = rgb_grid[y][x]
                row_data.extend((b, g, r)) # BGR
            if padding > 0:
                row_data.extend(b"\x00" * padding)
            f.write(row_data)

def parse_font_slice(slice_data):
    magic = slice_data[:13]
    if magic != b"MHIDS Fuente\x1a":
        raise ValueError("Cabecera mágica no válida")

    file_type, glyph_height = struct.unpack_from("<HH", slice_data, 13)
    cursor = 17
    
    # Calcular cantidad de glifos disponibles
    # En fractus.fon extendido son 120 glifos
    # Leemos el primer offset para determinar cuántos elementos hay en la tabla
    first_width, first_offset = struct.unpack_from("<HI", slice_data, cursor)
    num_glyphs = (first_offset - 17) // 6

    glyphs = []
    cursor = 17
    for i in range(num_glyphs):
        w, goff = struct.unpack_from("<HI", slice_data, cursor)
        cursor += 6
        gbytes = slice_data[goff : goff + w * glyph_height]
        grid = []
        for r in range(glyph_height):
            row = [gbytes[r * w + c] for c in range(w)]
            grid.append(row)
        glyphs.append({"width": w, "grid": grid})

    return file_type, glyph_height, glyphs

def parse_archive(fon_path):
    with open(fon_path, "rb") as f:
        data = f.read()

    # Layouts compatibles según tamaño del archivo
    if len(data) == 78487:
        slices = {
            "france": (16685, 41085),
            "arial": (0, 16685),
            "small": (57770, 5940),
            "courier": (63710, 14777),
        }
    elif len(data) == 78170:
        slices = {
            "france": (16631, 40833),
            "arial": (0, 16631),
            "small": (57464, 5929),
            "courier": (63393, 14777),
        }
    else:
        # Layout legado original 97 glifos
        slices = {
            "france": (13685, 33891),
            "arial": (0, 13685),
            "small": (47576, 4746),
            "courier": (52322, 11948),
        }

    faces = {}
    for name, (off, sz) in slices.items():
        slice_data = data[off : off + sz]
        ftype, h, glyphs = parse_font_slice(slice_data)
        faces[name] = {"file_type": ftype, "height": h, "glyphs": glyphs}

    return faces

def export_font_to_bmp(name, face, out_path, cols=12):
    H = face["height"]
    glyphs = face["glyphs"]
    N = len(glyphs)
    rows = (N + cols - 1) // cols
    
    max_w = max(g["width"] for g in glyphs)
    
    cell_w = max_w + 4
    cell_h = H + 4
    margin = 8
    
    img_w = margin * 2 + cols * cell_w
    img_h = margin * 2 + rows * cell_h
    
    COLOR_BG = (0, 0, 0)
    COLOR_BORDER = (128, 128, 128)
    COLOR_GLYPH = (255, 255, 255)
    
    # Inicializar matriz de píxeles
    rgb_grid = [[COLOR_BG for _ in range(img_w)] for _ in range(img_h)]
    
    for idx, g in enumerate(glyphs):
        col_idx = idx % cols
        row_idx = idx // cols
        
        bx0 = margin + col_idx * cell_w + 1
        by0 = margin + row_idx * cell_h + 1
        
        gw = g["width"]
        grid = g["grid"]
        
        bx1 = bx0 + gw + 1
        by1 = by0 + H + 1
        
        # Dibujar rectángulo delimitador
        for x in range(bx0, bx1 + 1):
            rgb_grid[by0][x] = COLOR_BORDER
            rgb_grid[by1][x] = COLOR_BORDER
        for y in range(by0, by1 + 1):
            rgb_grid[y][bx0] = COLOR_BORDER
            rgb_grid[y][bx1] = COLOR_BORDER
            
        # Dibujar glifo en blanco
        for r in range(H):
            for c in range(gw):
                if grid[r][c] != 0:
                    rgb_grid[by0 + 1 + r][bx0 + 1 + c] = COLOR_GLYPH
                    
    if HAS_PIL:
        img = Image.new("RGB", (img_w, img_h), COLOR_BG)
        pixels = img.load()
        for y in range(img_h):
            for x in range(img_w):
                pixels[x, y] = rgb_grid[y][x]
        img.save(out_path, format="BMP")
    else:
        write_bmp_24bpp(out_path, img_w, img_h, rgb_grid)

    print(f"Extraído: {out_path} ({img_w}x{img_h} px, {N} glifos)")

def main():
    parser = argparse.ArgumentParser(description="Extrae tipografías de fractus.fon a bitmaps BMP")
    parser.add_argument("--fon", default="", help="Ruta al archivo fractus.fon")
    parser.add_argument("--out-dir", default="", help="Directorio de salida para los bitmaps")
    parser.add_argument("--cols", type=int, default=12, help="Columnas en la cuadrícula (por defecto 12)")
    args = parser.parse_args()

    # Resolver rutas por defecto
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, "..", ".."))

    fon_path = args.fon
    if not fon_path:
        candidates = [
            os.path.join(project_root, "fractus.fon"),
            os.path.join(os.getcwd(), "fractus.fon"),
            "fractus.fon"
        ]
        for c in candidates:
            if os.path.exists(c):
                fon_path = c
                break

    if not fon_path or not os.path.exists(fon_path):
        print(f"Error: No se encontró fractus.fon en las rutas esperadas.")
        sys.exit(1)

    out_dir = args.out_dir
    if not out_dir:
        out_dir = os.path.join(project_root, "src", "fonts")

    os.makedirs(out_dir, exist_ok=True)
    print(f"Leyendo fuente desde: {fon_path}")
    print(f"Directorio de salida: {out_dir}")

    faces = parse_archive(fon_path)
    for font_name in ["arial", "france", "small", "courier"]:
        out_file = os.path.join(out_dir, f"{font_name}.bmp")
        export_font_to_bmp(font_name, faces[font_name], out_file, cols=args.cols)

    print("Extracción completada con éxito.")

if __name__ == "__main__":
    main()
