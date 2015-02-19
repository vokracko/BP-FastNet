CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3 -O0

.PHONY: lpm

all: test-lpm

bspl: lib-bspl lpm-bspl

tbm: lib-tbm lpm-tbm

lib-bspl:
	make -C ../lib bspl

lib-tbm:
	make -C ../lib tbm

test-lpm:
	make test-bspl
	make test-tbm

test-tbm: lib-tbm lpm-tbm
	./lpm.sh lpm-tbm

test-bspl: lib-bspl lpm-bspl
	./lpm.sh lpm-bspl

lpm-bspl: lib-bspl cmd.c
	$(CC) $(FLAGS) cmd.c ../lib/lpm.a -o lpm-bspl -pthread

lpm-tbm: lib-tbm cmd.c
	$(CC) $(FLAGS) cmd.c ../lib/lpm.a -o lpm-tbm -pthread

clean:
	rm -rf lpm-* *.o 2>/dev/null

.FORCE:
