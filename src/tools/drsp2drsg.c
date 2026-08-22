#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/formats.h"

static void print_usage(const char *program_name)
{
    fprintf(stderr, "Uso: %s <fichero.drsg> <paleta.drsp> <salida.drsg>\n", program_name);
    fprintf(stderr, "Modifica la paleta de un grafico DRSG con una paleta DRSP y guarda el resultado en un segundo DRSG.\n");
}

int main(int argc, char **argv)
{
    const char *graphic_input;
    const char *palette_input;
    const char *output_path;
    fractus_indexed_image image;
    fractus_palette graphic_palette;
    fractus_palette new_palette;
    uint16_t video_mode;
    fractus_status status;

    if (argc < 4 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage((argc > 0 && argv[0] != NULL) ? argv[0] : "drsp2drsg");
        return (argc < 4) ? 1 : 0;
    }

    graphic_input = argv[1];
    palette_input = argv[2];
    output_path = argv[3];

    /* If user passed arguments in order <paleta.drsp> <fichero.drsg> <salida.drsg>, auto-detect by extension */
    if (fractus_formats_has_extension(graphic_input, ".drsp") ||
        fractus_formats_has_extension(graphic_input, ".mhp") ||
        fractus_formats_has_extension(palette_input, ".drsg") ||
        fractus_formats_has_extension(palette_input, ".mhg")) {
        const char *temp = graphic_input;
        graphic_input = palette_input;
        palette_input = temp;
    }

    memset(&image, 0, sizeof(image));
    memset(&graphic_palette, 0, sizeof(graphic_palette));
    memset(&new_palette, 0, sizeof(new_palette));

    /* Load input graphic */
    status = fractus_legacy_graphic_load(graphic_input, &image, &graphic_palette, &video_mode);
    if (status != FRACTUS_STATUS_OK) {
        fprintf(stderr, "Error: No se pudo cargar el archivo DRSG '%s'\n", graphic_input);
        return 1;
    }

    /* Load new palette */
    status = fractus_legacy_palette_load(palette_input, &new_palette);
    if (status != FRACTUS_STATUS_OK) {
        fprintf(stderr, "Error: No se pudo cargar la paleta DRSP '%s'\n", palette_input);
        fractus_indexed_image_shutdown(&image);
        return 1;
    }

    /* Save modified graphic with new palette */
    status = fractus_legacy_graphic_save(output_path, &image, &new_palette);
    if (status != FRACTUS_STATUS_OK) {
        fprintf(stderr, "Error: No se pudo guardar el nuevo archivo DRSG '%s'\n", output_path);
        fractus_indexed_image_shutdown(&image);
        return 1;
    }

    printf("Grafico modificado con exito: '%s' + paleta '%s' -> '%s'\n",
        graphic_input, palette_input, output_path);

    fractus_indexed_image_shutdown(&image);
    return 0;
}
