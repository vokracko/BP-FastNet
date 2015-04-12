CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3
ALG=dfa
GENERAL=../general

all: FLAGS=-std=c99 -Wall -Wextra -g3 -DNDEBUG
all: lib

lib: list $(ALG).c precompile-headers
	$(CC) $(FLAGS) -D$(ALG)=1 -c $(ALG).c -o $(ALG).o
	rm -f regex.a
	ar rcs regex.a $(ALG).o  $(GENERAL)/list.o

list:
	make -C $(GENERAL) list

precompile-headers: types.h
	gcc -D$(ALG) -E -dU -nostdinc types.h -o types-precompiled.h

clean:
	rm -rf *.o *.a *.so 2>/dev/null
