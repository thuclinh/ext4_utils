CC=gcc
LINK_OPTS=-ggdb
COMP_OPTS=-ggdb -Wall -std=c99
RM=rm -f

all: alloc test_size2ull

clean:
	$(RM) alloc test_size2ull *.o

alloc: alloc.o
	$(CC) -o alloc alloc.o $(LINK_OPTS)

alloc.o: alloc.c
	$(CC) -c alloc.c $(COMP_OPTS)
	
test_size2ull: test_size2ull.o size2ull.o
	$(CC) -o test_size2ull test_size2ull.o size2ull.o $(LINK_OPTS)
	
test_size2ull.o: test_size2ull.c
	$(CC) -c test_size2ull.c $(COMP_OPTS)

size2ull.o:
	$(CC) -c size2ull.c $(COMP_OPTS)