#include <stdio.h>
#include <stdlib.h>
#include "../table/table.h"
#include "plugin.h"

// Handler do comando count_rows
void count_rows_handler(struct table **current_table, char *args)
{
    if (!*current_table)
    {
        printf("Error: No table is currently loaded.\n");
        return;
    }

    printf("Table has %lu rows and %lu columns.\n",
           (unsigned long)(*current_table)->num_rows,
           (unsigned long)(*current_table)->num_cols);
}

// Estrutura do plugin
static struct command_plugin count_rows_plugin = {
    .name = "count_rows",
    .description = "displays the number of rows and columns in the table",
    .handler = count_rows_handler
};

// Função de inicialização que o main irá chamar
struct command_plugin *plugin_init(void)
{
    return &count_rows_plugin;
}
