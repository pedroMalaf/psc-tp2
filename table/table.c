#include <stdio.h>
#include <stddef.h>
#include <csv.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "table.h"

struct load_context
{
    struct table *table;
    size_t current_row;
    size_t current_col;
    char **temp_row;
};

// função auxiliar para impedir que espaços sejam removidos
int is_not_space(unsigned char c)
{
    return 0;
}

// função auxiliar para processar cada célula lida
// quando uma célula é lida, esta função é chamada
void process_cell(void *s, size_t len, void *data)
{
    struct load_context *ctx = (struct load_context *)data;

    // se estivermos a ler a primeira linha, ainda não sabemos o número de colunas
    // vamos alocando memória até sabermos o número de colunas correto
    if (ctx->current_row == 0)
    {
        if (ctx->current_col >= MAX_COLS)
            return;

        // realoca o array temporário para aceitar mais 1 coluna
        // deste modo não desperdiçamos memória. vamos alocando memória até ao  número de colunas correto
        char **new_row = realloc(ctx->temp_row, (ctx->current_col + 1) * sizeof(char *));
        ctx->temp_row = new_row;
    }

    // alocar memória exata para o tamanho da string da cel (+1 para o terminador de string '\0')
    char *cell_content = malloc(len + 1);
    if (cell_content)
    {
        // copiar o conteúdo da célula para a nova string
        memcpy(cell_content, s, len);
        cell_content[len] = '\0';
        // guardar o conteúdo da célula na linha temporária
        ctx->temp_row[ctx->current_col] = cell_content;
    }

    ctx->current_col++;
}

// função auxiliar para libertar a memória associada a uma tabela
void table_free(struct table *t)
{
    if (t == NULL)
        return;

    // Percorrer todas as linhas
    for (size_t i = 0; i < t->num_rows; i++)
    {
        // Se a linha existe
        if (t->data[i] != NULL)
        {
            // Percorrer todas as colunas dessa linha e libertar as strings
            for (size_t j = 0; j < t->num_cols; j++)
            {
                free(t->data[i][j]);
            }
            // Libertar o array de ponteiros da linha
            free(t->data[i]);
        }
    }

    // Libertar a matriz principal (array de linhas)
    free(t->data);

    // Libertar a estrutura
    free(t);
}

// função auxiliar para duplicar uma linha da tabela
// utilizada na função table_filter
// como precisamos de retornar uma nova tabela na função table_filter precisamos de duplicar as linhas que satisfazem o predicado
static char **duplicate_row(size_t num_cols, char **src_row)
{
    char **new_row = malloc(num_cols * sizeof(char *));
    if (!new_row)
        return NULL;

    for (size_t i = 0; i < num_cols; i++)
    {
        // Aloca e copia a string de cada célula
        // Se src_row[i] for NULL (célula vazia), mantemos NULL ou string vazia
        if (src_row[i])
        {
            new_row[i] = strdup(src_row[i]); // strdup aloca e copia
            if (!new_row[i])
            {
                // Em caso de falha a meio, limpar o que já foi feito
                for (size_t k = 0; k < i; k++)
                    free(new_row[k]);
                free(new_row);
                return NULL;
            }
        }
        else
        {
            new_row[i] = NULL;
        }
    }
    return new_row;
}

// quando uma linha termina, esta função é chamada
void process_row(int c, void *data)
{
    struct load_context *ctx = (struct load_context *)data;

    // se acabamos de ler a primeira linha, definimos o número de colunas da tabela
    if (ctx->current_row == 0)
    {
        ctx->table->num_cols = ctx->current_col;
    }

    // verificar se precisamos de aumentar a capacidade do array de ponteiros para linhas
    if (ctx->table->num_rows >= ctx->table->pointer_array_capacity)
    {
        // duplicamos a capacidade do array de ponteiros caso seja necessário
        size_t new_capacity = ctx->table->pointer_array_capacity * 2;
        if (new_capacity == 0)
            new_capacity = INITIAL_POINTER_ARR_CAPACITY;

        // realocar a memória de data para a nova capacidade
        // char ** porque é um array de ponteiros para linhas
        char ***new_data = realloc(ctx->table->data, new_capacity * sizeof(char **));

        // atualizar o ponteiro da tabela para a nova área de memória e a capacidade
        ctx->table->data = new_data;
        ctx->table->pointer_array_capacity = new_capacity;
    }

    // adicionar a linha temporária à tabela
    ctx->table->data[ctx->table->num_rows] = ctx->temp_row;
    // incrementar o número de linhas na tabela
    ctx->table->num_rows++;
    // preparar para a próxima linha
    ctx->current_row++;
    // meter a coluna atual a zero para a próxima linha
    ctx->current_col = 0;

    // alocar uma nova linha temporária para a próxima linha
    if (ctx->table->num_cols > 0)
    {
        // como já sabemos o tamanho exato de colunas da tabela
        // alocamos linha temporária com o tamanho correto
        ctx->temp_row = malloc(ctx->table->num_cols * sizeof(char *));
        if (ctx->temp_row)
            // inicializar a linha temporária a zero
            memset(ctx->temp_row, 0, ctx->table->num_cols * sizeof(char *));
    }
    else
    {
        ctx->temp_row = NULL;
    }
}

// função para carregar uma tabela a partir de um ficheiro CSV
struct table *table_load_csv(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
        return NULL;

    // alocar a estrutura da tabela
    struct table *t = malloc(sizeof(struct table));
    if (!t)
    {
        fclose(fp);
        return NULL;
    }

    // inicializar a estrutura da tabela
    t->num_rows = 0;
    t->num_cols = 0;
    t->pointer_array_capacity = INITIAL_POINTER_ARR_CAPACITY;
    t->data = malloc(t->pointer_array_capacity * sizeof(char **));

    if (!t->data)
    {
        free(t);
        fclose(fp);
        return NULL;
    }

    // inicializar o parser CSV
    struct csv_parser p;
    if (csv_init(&p, 0) != 0)
    {
        free(t->data);
        free(t);
        fclose(fp);
        return NULL;
    }

    // configurar o parser para não remover espaços em branco
    csv_set_space_func(&p, is_not_space);

    // inicializar o contexto de carregamento
    // aponta para a tabela que estamos a preencher
    struct load_context ctx;
    ctx.table = t;
    ctx.current_col = 0;
    ctx.current_row = 0;
    ctx.temp_row = NULL;

    char buf[1024];
    size_t bytes_read;

    // ler o ficheiro em blocos e processar o CSV
    // cada célula e linha lida invoca as funções de callback definidas (process_cell e process_row)
    while ((bytes_read = fread(buf, 1, 1024, fp)) > 0)
    {
        if (csv_parse(&p, buf, bytes_read, process_cell, process_row, &ctx) != bytes_read)
        {
            fprintf(stderr, "erro ao processar csv: %s\n", csv_strerror(csv_error(&p)));
            break;
        }
    }

    csv_fini(&p, process_cell, process_row, &ctx);

    // aqui estamos a libertar memória alocada porque na função process_row é alocada sempre memória para a próxima row
    // quando chegamos ao fim do ficheiro e não temos mais rows para ler, ele aloca à mesma memória para a próxima row
    if (ctx.temp_row != NULL)
    {
        free(ctx.temp_row);
    }

    csv_free(&p);
    fclose(fp);

    return t;
}

// função para salvar uma tabela num ficheiro CSV
void table_save_csv(const struct table *table, const char *filename)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp)
        return;

    for (size_t i = 0; i < table->num_rows; i++)
    {
        for (size_t j = 0; j < table->num_cols; j++)
        {
            char *str = table->data[i][j];
            int needs_quotes = 0;
            size_t len = strlen(str);

            // Verificar se a célula precisa de aspas
            // (Se tiver virgulas, aspas, ou quebras de linha)
            for (size_t k = 0; k < len; k++)
            {
                if (str[k] == ',' || str[k] == '"' || str[k] == '\n' || str[k] == '\r')
                {
                    needs_quotes = 1;
                    break;
                }
            }

            if (needs_quotes)
            {
                fputc('"', fp); // Abre aspas
                for (size_t k = 0; k < len; k++)
                {
                    if (str[k] == '"')
                    {
                        fputc('"', fp); // Escapa aspa dupla (" torna-se "")
                    }
                    fputc(str[k], fp);
                }
                fputc('"', fp); // Fecha aspas
            }
            else
            {
                // Se não tiver caracteres especiais, escreve normalmente
                fprintf(fp, "%s", str);
            }

            // Adicionar separador se não for a última coluna
            if (j < table->num_cols - 1)
                fprintf(fp, ",");
        }
        // Fim da linha
        fprintf(fp, "\n");
    }

    fclose(fp);
}

// função para filtrar uma tabela com base num predicado
struct table *table_filter(const struct table *table,
                           bool (*predicate)(const void *row, const void *context),
                           const void *context)
{
    // Alocar a estrutura da nova tabela
    struct table *new_table = malloc(sizeof(struct table));
    if (!new_table)
        return NULL;

    // Inicializar metadados
    new_table->num_cols = table->num_cols;
    new_table->num_rows = 0;
    new_table->pointer_array_capacity = INITIAL_POINTER_ARR_CAPACITY;
    new_table->data = malloc(new_table->pointer_array_capacity * sizeof(char **));

    if (!new_table->data)
    {
        free(new_table);
        return NULL;
    }

    // Iterar sobre as linhas da tabela original
    for (size_t i = 0; i < table->num_rows; i++)
    {
        // Obter a linha atual
        char **current_row = table->data[i];
        // Verificar se a linha satisfaz o predicado
        if (predicate((const void *)current_row, context))
        {
            // Verificar se precisamos de aumentar a capacidade do array de ponteiros para linhas
            // duplicamos a capacidade se necessário
            if (new_table->num_rows >= new_table->pointer_array_capacity)
            {
                size_t new_cap = new_table->pointer_array_capacity * 2;
                char ***new_data_ptr = realloc(new_table->data, new_cap * sizeof(char **));

                if (!new_data_ptr)
                {
                    table_free(new_table);
                    return NULL;
                }
                new_table->data = new_data_ptr;
                new_table->pointer_array_capacity = new_cap;
            }

            // Duplicar a linha
            char **row_copy = duplicate_row(new_table->num_cols, current_row);
            if (!row_copy)
            {
                table_free(new_table);
                return NULL;
            }

            // Adicionar a linha duplicada à nova tabela
            new_table->data[new_table->num_rows] = row_copy;
            new_table->num_rows++;
        }
    }

    return new_table;
}
// função para eliminar uma linha da tabela
int table_delete_row(struct table *table, size_t row_index)
{
    if (!table || row_index >= table->num_rows)
        return -1;

    // Libertar a memória da linha a eliminar
    if (table->data[row_index])
    {
        for (size_t j = 0; j < table->num_cols; j++)
        {
            free(table->data[row_index][j]);
        }
        free(table->data[row_index]);
    }

    // Mover todas as linhas seguintes uma posição para cima
    for (size_t i = row_index; i < table->num_rows - 1; i++)
    {
        table->data[i] = table->data[i + 1];
    }

    // Decrementar o número de linhas
    table->num_rows--;

    return 0;
}
