CC=gcc
STRIDE=2
FLAGS=-std=c99 -Wall -Wextra -g3
ALG=ac

all: FLAGS=-std=c99 -Wall -Wextra -g3 -DNDEBUG
all: lib

lib: $(ALG).c precompile-headers
	$(CC) $(FLAGS) -D$(ALG)=1 -c $(ALG).c -o $(ALG).o
	rm -f pm.a
	ar rcs pm.a $(ALG).o

precompile-headers: types.h
	gcc -D$(ALG) -E -dU -nostdinc types.h -o types-precompiled.h

clean:
	rm -rf *.o *.a *.so 2>/dev/null
