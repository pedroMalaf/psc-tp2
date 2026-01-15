# Ex4 - Sistema de Plugins para Tabelas CSV

Este exercício estende o programa de manipulação de tabelas CSV com um sistema de plugins dinâmicos.

## Funcionalidades

### Comandos Base
- `help` - Lista todos os comandos disponíveis
- `exit` - Sai do programa
- `load <filename>` - Carrega uma tabela CSV
- `save <filename>` - Guarda a tabela num ficheiro CSV
- `show <col><row>:<col><row>` - Mostra uma sub-tabela (ex: `show A1:B5`)
- `filter <column> <data>` - Filtra linhas pela coluna

### Sistema de Plugins
- `command <libfile>` - Carrega um novo comando a partir de um shared object

## Plugins de Exemplo

### delete_row
O plugin `libdelete_row.so` adiciona o comando:
- `delete_row <número_da_linha>` - Elimina uma linha da tabela

### count_rows
O plugin `libcount_rows.so` adiciona o comando:
- `count_rows` - Mostra o número de linhas e colunas da tabela

## Como Usar

1. Compilar tudo:
```bash
make
```

2. Executar o programa:
```bash
make run
```

3. Exemplo de sessão:
```
> load ../csv_files/frutas.csv
Table loaded successfully.

> show A1:C3
fornecedor    fruta   quantidade
joaquim maca    20
manuel  melancia        100

> command ./libcount_rows.so
Plugin 'count_rows' loaded successfully.

> count_rows
Table has 7 rows and 5 columns.

> command ./libdelete_row.so
Plugin 'delete_row' loaded successfully.

> help
List of available commands:
...
Loaded plugin commands:
count_rows                  - displays the number of rows and columns in the table
delete_row                  - deletes a row from the table by row number

> delete_row 2
Row 2 deleted successfully. Table now has 6 rows.

> count_rows
Table has 6 rows and 5 columns.

> show A1:C3
fornecedor    fruta   quantidade
manuel  melancia        100
humberto         amora   10

> save output.csv
Table saved to output.csv

> exit
Exiting program
```

## Estrutura de Ficheiros

```
ex4/
├── main.c                  - Programa principal com suporte a plugins
├── plugin.h                - Interface para plugins
├── delete_row.c            - Plugin para eliminar linhas
├── count_rows.c            - Plugin para contar linhas/colunas
├── libdelete_row.so        - Shared object do plugin delete_row (gerado)
├── libcount_rows.so        - Shared object do plugin count_rows (gerado)
├── main                    - Executável principal (gerado)
├── makefile                - Makefile para compilar tudo
└── README.md               - Este ficheiro
```

## Criar Novos Plugins

Para criar um novo plugin:

1. Criar um ficheiro `.c` com a estrutura:

```c
#include <stdio.h>
#include "../table/table.h"
#include "plugin.h"

void meu_handler(struct table **current_table, char *args)
{
    // Implementação do comando
}

static struct command_plugin meu_plugin = {
    .name = "meu_comando",
    .description = "descrição do comando",
    .handler = meu_handler
};

struct command_plugin *plugin_init(void)
{
    return &meu_plugin;
}
```

2. Compilar como shared object:
```bash
gcc -Wall -g -fPIC -I../table -shared -o libmeu_plugin.so meu_plugin.c
```

3. Carregar no programa:
```
> command ./libmeu_plugin.so
```

## Notas Técnicas

- O sistema usa `dlopen()` e `dlsym()` para carregamento dinâmico
- Cada plugin deve exportar uma função `plugin_init()` que retorna um ponteiro para `struct command_plugin`
- O handler recebe um ponteiro para o ponteiro da tabela atual (`struct table **`) para poder modificá-la
- Máximo de 20 plugins podem ser carregados simultaneamente
