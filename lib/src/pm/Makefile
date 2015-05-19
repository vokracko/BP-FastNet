CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3
ALG=ac
COMMON=../common

all: FLAGS=-std=c99 -Wall -Wextra -DNDEBUG
all: lib

lib: queue $(ALG).c precompile-headers
	$(CC) $(FLAGS) -DALG_$(ALG) -c $(ALG).c -o $(ALG).o
	rm -f pm.a
	ar rcs pm.a $(ALG).o $(COMMON)/queue.o

queue:
	make -C $(COMMON) queue

precompile-headers: types.h
	$(CC) -DALG_$(ALG) -E -dU -nostdinc types.h -o types-precompiled.h

clean:
	rm -rf *.o *.a *.so types-precompiled.h 2>/dev/null
