CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3
COMMON=../common

all: FLAGS=-std=c99 -Wall -Wextra -DNDEBUG
all: lib

lib: queue dfa.c precompile-headers
	$(CC) $(FLAGS) -c dfa.c -o dfa.o
	rm -f regex.a
	ar rcs regex.a dfa.o $(COMMON)/queue.o

queue:
	make -C $(COMMON) queue

precompile-headers: types.h
	$(CC) -DALG_$(ALG) -E -dU -nostdinc types.h -o types-precompiled.h

clean:
	rm -rf *.o *.a *.so types-precompiled.h 2>/dev/null
