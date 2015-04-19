CC=gcc
STRIDE=2
FLAGS=-std=c99 -Wall -Wextra -g3
ALG=ac
GENERAL=../general

all: FLAGS=-std=c99 -Wall -Wextra -g3 -DNDEBUG
all: lib

lib: list $(ALG).c precompile-headers
	$(CC) $(FLAGS) -DALG_$(ALG) -c $(ALG).c -o $(ALG).o
	rm -f pm.a
	ar rcs pm.a $(ALG).o $(GENERAL)/list.o

list:
	make -C $(GENERAL) list

precompile-headers: types.h
	gcc -DALG_$(ALG) -E -dU -nostdinc types.h -o types-precompiled.h

clean:
	rm -rf *.o *.a *.so 2>/dev/null
