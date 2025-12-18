#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../table/table.h"

struct table *current_table = NULL;

#define MAX_CMD_LEN 1024

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
    return 0;
}

void list_commands()
{
    printf("List of available commands:\n");
    printf("exit                        -exits the program\n");
    printf("load <filename>             -loads the content of the file <filename> to the table\n");
    printf("save <filename>             -saves the table on the file <filename>\n");
    printf("show <col><row>:<col><row>  -shows the content of the table defined by the given coordinates\n");
}

void load_table(char *args)
{
    if (!args)
    {
        printf("Error: you need to intruduce the file name you want to load a table from");
    }

    args[strcspn(args, "\n")] = 0;

    clear_current_table();
    current_table = table_load_csv(args);

    if (current_table)
    {
        printf("Table loaded successfuly.\n");
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
    }
    if (!args)
    {
        printf("Error: You need to intruduce the file name you want to save the table to\n");
    }

    args[strcspn(args, "\n")] = 0;
    table_save_csv(current_table, args);
    printf("Table loaded to %s\n", args);
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
    // " %c" com espaço antes ignora whitespace inicial
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
            printf("exiting program\n");
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
        default:
            printf("Unkown command: %s\n", cmd);
        }
    }
    clear_current_table();
    return 0;
}