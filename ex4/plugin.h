#ifndef PLUGIN_H
#define PLUGIN_H

#include "../table/table.h"

// Estrutura para representar um plugin de comando
struct command_plugin
{
    const char *name;        // Nome do comando
    const char *description; // Descrição do comando
    void (*handler)(struct table **current_table, char *args); // Função que executa o comando
};

// Função que cada plugin deve exportar
// Retorna um ponteiro para a estrutura do plugin
typedef struct command_plugin *(*plugin_init_func)(void);

#endif
