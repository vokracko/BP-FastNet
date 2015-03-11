CC=gcc
STRIDE=2
FLAGS=-std=c99 -Wall -Wextra -g3
ALG=aho-corasic

all: lib

lib: $(ALG).c
	$(CC) $(FLAGS) -c $(ALG).c -o $(ALG).o
	rm -f pm.a
	ar rcs pm.a $(ALG).o

clean:
	rm -rf *.o *.a *.so 2>/dev/null
