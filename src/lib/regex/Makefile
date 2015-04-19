CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3
ALG=dfa
GENERAL=../general

all: FLAGS=-std=c99 -Wall -Wextra -g3 -DNDEBUG
all: lib

lib: stack $(ALG).c precompile-headers
	$(CC) $(FLAGS) -DALG_$(ALG) -c $(ALG).c -o $(ALG).o
	rm -f regex.a
	ar rcs regex.a $(ALG).o $(GENERAL)/stack.o

stack:
	make -C $(GENERAL) stack

precompile-headers: types.h
	gcc -DALG_$(ALG) -E -dU -nostdinc types.h -o types-precompiled.h

clean:
	rm -rf *.o *.a *.so 2>/dev/null
