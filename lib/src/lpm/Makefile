CC=gcc
STRIDE=2
FLAGS=-std=c99 -Wall -Wextra -g3
ALG=tbm

all: FLAGS=-std=c99 -Wall -Wextra -g3 -DNDEBUG
all: lib

lib: $(ALG).c precompile-headers
	$(CC) $(FLAGS) -DALG_$(ALG)=1 -DSTRIDE=$(STRIDE) -c $(ALG).c -o $(ALG).o
	rm -f lpm.a
	ar rcs lpm.a $(ALG).o

precompile-headers: types.h
	$(CC) -DALG_$(ALG) -DSTRIDE=$(STRIDE) -E -dU -nostdinc types.h -o types-precompiled.h

clean:
	rm -rf *.o *.a *.so types-precompiled.h 2>/dev/null
