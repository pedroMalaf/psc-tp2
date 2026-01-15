#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <dlfcn.h>
#include "../table/table.h"
#include "plugin.h"

struct table *current_table = NULL;

#define MAX_CMD_LEN 1024
#define MAX_PLUGINS 20

// Array de plugins carregados dinamicamente
struct command_plugin *loaded_plugins[MAX_PLUGINS];
void *plugin_handles[MAX_PLUGINS]; // Handles para dlclose
int num_plugins = 0;

// Estrutura para passar dados ao filtro
struct filter_ctx
{
    int col_idx;       // Índice da coluna a verificar
    char *search_term; // Texto que queremos encontrar
};

// Função Predicado: Retorna true se a linha deve ser mantida
bool filter_equals(const void *row_ptr, const void *context)
{
    char **row = (char **)row_ptr;
    const struct filter_ctx *ctx = (const struct filter_ctx *)context;

    if (!row[ctx->col_idx])
        return false;

    // Compara o conteúdo da célula com o termo de pesquisa
    return strcmp(row[ctx->col_idx], ctx->search_term) == 0;
}

int get_col_index(char c)
{
    if (c >= 'a' && c <= 'z')
        return c - 'a';
    if (c >= 'A' && c <= 'Z')
        return c - 'A';
    return -1;
}

void clear_current_table()
{
    if (current_table)
    {
        table_free(current_table);
        current_table = NULL;
    }
}

int get_command(const char *cmd)
{
    if (strcmp(cmd, "help") == 0)
        return 1;
    if (strcmp(cmd, "exit") == 0)
        return 2;
    if (strcmp(cmd, "load") == 0)
        return 3;
    if (strcmp(cmd, "save") == 0)
        return 4;
    if (strcmp(cmd, "show") == 0)
        return 5;
    if (strcmp(cmd, "filter") == 0)
        return 6;
    if (strcmp(cmd, "command") == 0)
        return 7;
    return 0;
}

void list_commands()
{
    printf("List of available commands:\n");
    printf("exit                        - exits the program\n");
    printf("load <filename>             - loads the content of the file <filename> to the table\n");
    printf("save <filename>             - saves the table on the file <filename>\n");
    printf("show <col><row>:<col><row>  - shows the content of the table defined by the given coordinates\n");
    printf("filter <column> <data>      - eliminates the lines of the table with the content in <column> different from <data>\n");
    printf("command <libfile>           - loads a new command plugin from shared object <libfile>\n");
    
    // Listar plugins carregados
    if (num_plugins > 0)
    {
        printf("\nLoaded plugin commands:\n");
        for (int i = 0; i < num_plugins; i++)
        {
            printf("%-27s - %s\n", loaded_plugins[i]->name, loaded_plugins[i]->description);
        }
    }
}

void load_table(char *args)
{
    if (!args)
    {
        printf("Error: you need to introduce the file name you want to load a table from\n");
        return;
    }

    args[strcspn(args, "\n")] = 0;

    clear_current_table();
    current_table = table_load_csv(args);

    if (current_table)
    {
        printf("Table loaded successfully.\n");
    }
    else
    {
        printf("Error loading file %s\n", args);
    }
}

void save_table(char *args)
{
    if (!current_table)
    {
        printf("Error: No table is currently loaded\n");
        return;
    }
    if (!args)
    {
        printf("Error: You need to introduce the file name you want to save the table to\n");
        return;
    }

    args[strcspn(args, "\n")] = 0;
    table_save_csv(current_table, args);
    printf("Table saved to %s\n", args);
}

void show_sub_table(char *args)
{
    if (!current_table)
    {
        printf("Error: No table loaded.\n");
        return;
    }
    if (!args)
    {
        printf("Error: Missing coordinates. Usage: show A1:B5\n");
        return;
    }

    char c1, c2;
    int r1, r2;

    // Parse das coordenadas (ex: A1:B5)
    if (sscanf(args, " %c%d:%c%d", &c1, &r1, &c2, &r2) != 4)
    {
        printf("Invalid format. Usage: show A1:B5\n");
        return;
    }

    int col_start = get_col_index(c1);
    int col_end = get_col_index(c2);
    int row_start = r1 - 1;
    int row_end = r2 - 1;

    // Validação de limites
    if (col_start < 0 || col_end >= current_table->num_cols ||
        row_start < 0 || row_end >= current_table->num_rows)
    {
        printf("Coordinates out of bounds.\n");
        return;
    }

    // Imprimir subtabela
    for (int i = row_start; i <= row_end; i++)
    {
        for (int j = col_start; j <= col_end; j++)
        {
            char *val = current_table->data[i][j];
            printf("%s\t", val ? val : "");
        }
        printf("\n");
    }
}

void filter_table(char *args)
{
    if (!current_table)
    {
        printf("Error: No table is currently loaded.\n");
        return;
    }
    if (!args)
    {
        printf("Error: Usage filter <col> <value>\n");
        return;
    }

    // Separar a Coluna do Valor
    char *col_str = strtok(args, " ");
    char *val_str = strtok(NULL, "\n");

    if (!col_str || !val_str)
    {
        printf("Error: Invalid format. Usage: filter <col> <value>\n");
        return;
    }

    // Converter letra da coluna para índice
    int col_idx = get_col_index(col_str[0]);

    // Validar se a coluna existe
    if (col_idx < 0 || col_idx >= current_table->num_cols)
    {
        printf("Error: Invalid column '%s'.\n", col_str);
        return;
    }

    // Preparar o contexto para o filtro
    struct filter_ctx ctx;
    ctx.col_idx = col_idx;
    ctx.search_term = val_str;

    // Chamar a função da biblioteca
    struct table *new_table = table_filter(current_table, filter_equals, &ctx);

    if (new_table)
    {
        printf("Filter applied. Rows reduced from %lu to %lu.\n",
               (unsigned long)current_table->num_rows,
               (unsigned long)new_table->num_rows);

        // Substituir a tabela antiga pela nova
        clear_current_table();
        current_table = new_table;
    }
    else
    {
        printf("Error: Filter failed (memory or internal error).\n");
    }
}

void load_command_plugin(char *args)
{
    if (!args)
    {
        printf("Error: Usage: command <libfile>\n");
        return;
    }

    if (num_plugins >= MAX_PLUGINS)
    {
        printf("Error: Maximum number of plugins reached (%d)\n", MAX_PLUGINS);
        return;
    }

    args[strcspn(args, "\n")] = 0;

    // Carregar a biblioteca dinâmica
    void *handle = dlopen(args, RTLD_LAZY);
    if (!handle)
    {
        printf("Error loading plugin %s: %s\n", args, dlerror());
        return;
    }

    // Procurar a função plugin_init
    plugin_init_func init = (plugin_init_func)dlsym(handle, "plugin_init");
    char *error = dlerror();
    if (error != NULL)
    {
        printf("Error finding plugin_init in %s: %s\n", args, error);
        dlclose(handle);
        return;
    }

    // Chamar a função de inicialização
    struct command_plugin *plugin = init();
    if (!plugin)
    {
        printf("Error: plugin_init returned NULL\n");
        dlclose(handle);
        return;
    }

    // Verificar se já existe um plugin com este nome
    for (int i = 0; i < num_plugins; i++)
    {
        if (strcmp(loaded_plugins[i]->name, plugin->name) == 0)
        {
            printf("Error: Plugin '%s' already loaded\n", plugin->name);
            dlclose(handle);
            return;
        }
    }

    // Registrar o plugin
    loaded_plugins[num_plugins] = plugin;
    plugin_handles[num_plugins] = handle;
    num_plugins++;

    printf("Plugin '%s' loaded successfully.\n", plugin->name);
}

// Procura e executa um plugin
bool try_plugin_command(const char *cmd, char *args)
{
    for (int i = 0; i < num_plugins; i++)
    {
        if (strcmp(loaded_plugins[i]->name, cmd) == 0)
        {
            loaded_plugins[i]->handler(&current_table, args);
            return true;
        }
    }
    return false;
}

void cleanup_plugins()
{
    for (int i = 0; i < num_plugins; i++)
    {
        if (plugin_handles[i])
        {
            dlclose(plugin_handles[i]);
        }
    }
}

int main()
{
    char input[MAX_CMD_LEN];
    int is_running = 1;

    while (is_running)
    {
        printf("> ");

        if (!fgets(input, MAX_CMD_LEN, stdin))
        {
            break;
        }

        char *cmd = strtok(input, " \n");
        char *args = strtok(NULL, "");

        if (cmd == NULL)
        {
            continue;
        }

        switch (get_command(cmd))
        {
        case 1:
            list_commands();
            break;
        case 2:
            printf("Exiting program\n");
            is_running = 0;
            break;
        case 3:
            load_table(args);
            break;
        case 4:
            save_table(args);
            break;
        case 5:
            show_sub_table(args);
            break;
        case 6:
            filter_table(args);
            break;
        case 7:
            load_command_plugin(args);
            break;
        default:
            // Tentar executar como plugin
            if (!try_plugin_command(cmd, args))
            {
                printf("Unknown command: %s\n", cmd);
            }
            break;
        }
    }

    clear_current_table();
    cleanup_plugins();
    return 0;
}
