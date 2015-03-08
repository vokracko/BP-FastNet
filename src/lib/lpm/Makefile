CC=gcc
STRIDE=2
FLAGS=-std=c99 -Wall -Wextra -g3
ALG=tbm

all: lib

lib: $(ALG).c precompile-headers
	$(CC) $(FLAGS) -D$(ALG)=1 -DSTRIDE=$(STRIDE) -c $(ALG).c -o $(ALG).o
	rm -f lpm.a
	ar rcs lpm.a $(ALG).o

precompile-headers: types.h
	gcc -D$(ALG) -DSTRIDE=$(STRIDE) -E -dU -nostdinc types.h -o types-precompiled.h

aho-corasic: aho-corasic.c
	$(CC) $(FLAGS) -c aho-corasic.c -o aho-corasic.o
	rm -f aho-corasic.a
	ar rcs aho-corasic.a aho-corasic.o

clean:
	rm -rf *.o *.a *.so types-precompiled.h 2>/dev/null
