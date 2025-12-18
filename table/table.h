#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>
#include <stdbool.h>

#define MAX_COLS 26
#define INITIAL_POINTER_ARR_CAPACITY 10

struct table
{
    size_t num_cols;
    size_t num_rows;
    size_t pointer_array_capacity;
    char ***data;
};

// Carrega o CSV (Alínea b)
struct table *table_load_csv(const char *filename);

// Salva o CSV (Alínea c)
void table_save_csv(const struct table *table, const char *filename);

// Filtra a tabela (Alínea e)
struct table *table_filter(const struct table *table,
                           bool (*predicate)(const void *row, const void *context),
                           const void *context);

// Liberta memória
void table_free(struct table *t);

#endif