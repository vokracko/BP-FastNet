CC=gcc
STRIDE=2
FLAGS=-std=c99 -Wall -Wextra -g3 -O0

all: clean bspl

bspl: bspl.c
	$(CC) $(FLAGS) -c bspl.c -o bspl.o
	rm -f lpm.a
	ar rcs lpm.a bspl.o

tbm: tbm.c
	$(CC) $(FLAGS) -DSTRIDE=$(STRIDE) -c tbm.c -o tbm.o
	rm -f lpm.a
	ar rcs lpm.a tbm.o

clean:
	rm -rf *.o *.a *.so 2>/dev/null
