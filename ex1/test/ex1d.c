#include <stdio.h>
#include "table.h"

int main(int argc, char *argv[])
{
    // argv[0]: nome do programa
    // argv[1]: ficheiro destino (a criar)
    // argv[2]: ficheiro fonte (a ler)
    if (argc != 3)
    {
        fprintf(stderr, "Please do: %s <dest_file> <src_file>\n", argv[0]);
        return 1;
    }

    const char *dest_file = argv[1];
    const char *src_file = argv[2];

    struct table *table = table_load_csv(src_file);
    if (table == NULL)
    {
        fprintf(stderr, "ERROR loading file %s\n", src_file);
        return 1;
    }

    table_save_csv(table, dest_file);

    table_free(table);

    return 0;
}