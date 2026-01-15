CC = gcc
CFLAGS = -Wall -g -fPIC
INCLUDES = -I../table

# Caminho para a biblioteca do Ex2
LIB_PATH = ../ex2

# Nome do executável e plugins
TARGET = main
PLUGIN_DELETE_ROW = libdelete_row.so
PLUGIN_COUNT_ROWS = libcount_rows.so

all: $(TARGET) $(PLUGIN_DELETE_ROW) $(PLUGIN_COUNT_ROWS)

# Compilar o programa principal
# -ldl é necessário para usar dlopen/dlsym
$(TARGET): main.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) main.c -L$(LIB_PATH) -ltable -ldl

# Compilar o plugin delete_row como shared object
$(PLUGIN_DELETE_ROW): delete_row.c
	$(CC) $(CFLAGS) $(INCLUDES) -shared -o $(PLUGIN_DELETE_ROW) delete_row.c

# Compilar o plugin count_rows como shared object
$(PLUGIN_COUNT_ROWS): count_rows.c
	$(CC) $(CFLAGS) $(INCLUDES) -shared -o $(PLUGIN_COUNT_ROWS) count_rows.c

# Executar o programa
run: $(TARGET)
	export LD_LIBRARY_PATH=$(LIB_PATH):$$LD_LIBRARY_PATH && ./$(TARGET)

clean:
	rm -f $(TARGET) $(PLUGIN_DELETE_ROW) $(PLUGIN_COUNT_ROWS) *.o
