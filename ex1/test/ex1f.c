// ex1f.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"

#define FRUTAS_CSV "../csv_files/frutas.csv"
#define DEST_FILE_NAME "../csv_files/frutas_filtradas.csv"
#define PRICE_COL 4

bool price_lower_than_one_euro(const void *row_ptr, const void *context)
{
    char **row = (char **)row_ptr;
    int price_col_index = *(int *)context;

    if (!row[price_col_index])
        return false;

    double price = atof(row[price_col_index]);

    return price < 1.0;
}

int main()
{
    int price_col = PRICE_COL;

    struct table *tab = table_load_csv(FRUTAS_CSV);
    if (!tab)
    {
        fprintf(stderr, "Error loading %s\n", FRUTAS_CSV);
        return 1;
    }

    // Passamos o endereço de price_col como contexto
    struct table *filtered_tab = table_filter(tab, price_lower_than_one_euro, &price_col);

    if (!filtered_tab)
    {
        fprintf(stderr, "Erro ao filtrar a tabela (ou memória insuficiente).\n");
        table_free(tab);
        return 1;
    }

    table_save_csv(filtered_tab, DEST_FILE_NAME);

    table_free(tab);
    table_free(filtered_tab);

    return 0;
}