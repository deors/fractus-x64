#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/formats.h"

static void print_usage(const char *program_name)
{
    fprintf(stderr, "Uso: %s <fichero.drsg> [fichero.drsp]\n", program_name);
    fprintf(stderr, "Extrae la paleta de colores de un gráfico DRSG (o legacy MHG) y la guarda como DRSP.\n");
}

static void replace_or_append_extension(const char *input, const char *new_ext, char *output, size_t output_size)
{
    const char *last_dot = strrchr(input, '.');
    const char *last_slash = strrchr(input, '/');
    const char *last_backslash = strrchr(input, '\\');
    const char *path_sep = (last_slash > last_backslash) ? last_slash : last_backslash;

    if (last_dot != NULL && (path_sep == NULL || last_dot > path_sep)) {
        size_t base_len = (size_t)(last_dot - input);
        if (base_len + strlen(new_ext) < output_size) {
            memcpy(output, input, base_len);
            output[base_len] = '\0';
            strcat(output, new_ext);
            return;
        }
    }

    snprintf(output, output_size, "%s%s", input, new_ext);
}

int main(int argc, char **argv)
{
    const char *input_path;
    char output_path[512];
    fractus_indexed_image image;
    fractus_palette palette;
    uint16_t video_mode;
    fractus_status status;

    if (argc < 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage((argc > 0 && argv[0] != NULL) ? argv[0] : "drsg2drsp");
        return (argc < 2) ? 1 : 0;
    }

    input_path = argv[1];
    if (argc >= 3) {
        snprintf(output_path, sizeof(output_path), "%s", argv[2]);
    } else {
        replace_or_append_extension(input_path, ".drsp", output_path, sizeof(output_path));
    }

    memset(&image, 0, sizeof(image));
    memset(&palette, 0, sizeof(palette));

    status = fractus_legacy_graphic_load(input_path, &image, &palette, &video_mode);
    if (status != FRACTUS_STATUS_OK) {
        fprintf(stderr, "Error: No se pudo cargar el archivo DRSG '%s'\n", input_path);
        return 1;
    }

    status = fractus_legacy_palette_save(output_path, &palette);
    if (status != FRACTUS_STATUS_OK) {
        fprintf(stderr, "Error: No se pudo guardar la paleta DRSP '%s'\n", output_path);
        fractus_indexed_image_shutdown(&image);
        return 1;
    }

    printf("Paleta extraída con éxito: '%s' -> '%s'\n", input_path, output_path);

    fractus_indexed_image_shutdown(&image);
    return 0;
}
