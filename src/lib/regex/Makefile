CC=gcc
STRIDE=2
FLAGS=-std=c99 -Wall -Wextra -g3
ALG=dfa

all: lib

lib: $(ALG).c precompile-headers
	$(CC) $(FLAGS) -D$(ALG)=1 -c $(ALG).c -o $(ALG).o
	rm -f regex.a
	ar rcs regex.a $(ALG).o

precompile-headers: types.h
	gcc -D$(ALG) -E -dU -nostdinc types.h -o types-precompiled.h

clean:
	rm -rf *.o *.a *.so 2>/dev/null
