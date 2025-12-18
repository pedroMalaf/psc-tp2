CC = gcc
CFLAGS = -Wall -g
LIBS = -lcsv

INCLUDES = -I../table

SRC_TABLE = ../table/table.c
SRC_EX1D  = test/ex1d.c
SRC_EX1F  = test/ex1f.c

all: ex1d ex1f

table.o: $(SRC_TABLE)
	$(CC) $(CFLAGS) $(INCLUDES) -c $(SRC_TABLE) -o table.o

ex1d: $(SRC_EX1D) table.o
	$(CC) $(CFLAGS) $(INCLUDES) -o ex1d $(SRC_EX1D) table.o $(LIBS)

ex1f: $(SRC_EX1F) table.o
	$(CC) $(CFLAGS) $(INCLUDES) -o ex1f $(SRC_EX1F) table.o $(LIBS)

clean:
	rm -f *.o ex1d ex1f