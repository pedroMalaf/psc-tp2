CC = gcc
CFLAGS = -Wall -g
LIBS = -lcsv
INCLUDES = -I../table

SRC_TABLE = ../table/table.c
SRC_TEST  = ../ex1/test/ex1f.c

all: ex2

table.o: $(SRC_TABLE)
	$(CC) $(CFLAGS) -fPIC $(INCLUDES) -c $(SRC_TABLE) -o table.o

libtable.so: table.o
	$(CC) -shared -o libtable.so table.o $(LIBS)

ex2: $(SRC_TEST) libtable.so
	$(CC) $(CFLAGS) $(INCLUDES) -o ex2 $(SRC_TEST) -L. -ltable $(LIBS)

# Este alvo define a variável E executa o programa na mesma linha
run: ex2
	export LD_LIBRARY_PATH=.:$$LD_LIBRARY_PATH && ./ex2 ../frutas.csv saida_so.csv 4

clean:
	rm -f *.o *.so ex2