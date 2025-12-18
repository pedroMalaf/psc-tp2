CC = gcc
CFLAGS = -Wall -g
INCLUDES = -I../table

# Caminho para onde a biblioteca do Ex2 foi gerada
LIB_PATH = ../ex2

# Nome do executável
TARGET = main

all: $(TARGET)

# Compilar o shell
# -L$(LIB_PATH) diz para procurar bibliotecas na pasta ex2
# -ltable diz para usar o ficheiro libtable.so
$(TARGET): main.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) main.c -L$(LIB_PATH) -ltable

# Alvo para executar configurando a biblioteca
run: $(TARGET)
	export LD_LIBRARY_PATH=$(LIB_PATH):$$LD_LIBRARY_PATH && ./$(TARGET)

clean:
	rm -f $(TARGET) *.o