#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../table/table.h"
#include "plugin.h"

// Handler do comando delete_row
void delete_row_handler(struct table **current_table, char *args)
{
    if (!*current_table)
    {
        printf("Error: No table is currently loaded.\n");
        return;
    }

    if (!args)
    {
        printf("Error: Usage: delete_row <row_number>\n");
        return;
    }

    // Remover newline se existir
    args[strcspn(args, "\n")] = 0;

    // Converter argumento para número
    char *endptr;
    long row_num = strtol(args, &endptr, 10);

    // Verificar se a conversão foi bem-sucedida
    if (*endptr != '\0' || row_num < 1)
    {
        printf("Error: Invalid row number '%s'. Must be a positive integer.\n", args);
        return;
    }

    // Converter de número de linha (1-indexed) para índice (0-indexed)
    size_t row_index = (size_t)(row_num - 1);

    // Verificar se o índice está dentro dos limites
    if (row_index >= (*current_table)->num_rows)
    {
        printf("Error: Row %ld does not exist. Table has %lu rows.\n",
               row_num, (unsigned long)(*current_table)->num_rows);
        return;
    }

    // Eliminar a linha
    if (table_delete_row(*current_table, row_index) == 0)
    {
        printf("Row %ld deleted successfully. Table now has %lu rows.\n",
               row_num, (unsigned long)(*current_table)->num_rows);
    }
    else
    {
        printf("Error: Failed to delete row %ld.\n", row_num);
    }
}

// Estrutura do plugin
static struct command_plugin delete_row_plugin = {
    .name = "delete_row",
    .description = "deletes a row from the table by row number",
    .handler = delete_row_handler
};

// Função de inicialização que o main irá chamar
struct command_plugin *plugin_init(void)
{
    return &delete_row_plugin;
}
