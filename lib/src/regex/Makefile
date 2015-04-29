CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3
ALG=dfa
GENERAL=../general

all: FLAGS=-std=c99 -Wall -Wextra -g3 -DNDEBUG
all: lib

lib: queue dfa.c precompile-headers
	$(CC) $(FLAGS) -DALG_$(ALG) -c dfa.c -o dfa.o
	rm -f regex.a
	ar rcs regex.a dfa.o $(GENERAL)/queue.o

queue:
	make -C $(GENERAL) queue

precompile-headers: types.h
	gcc -DALG_$(ALG) -E -dU -nostdinc types.h -o types-precompiled.h

clean:
	rm -rf *.o *.a *.so 2>/dev/null
