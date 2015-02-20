CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3 -O0
LIB=../../lib/

all:
	./run.sh bspl && \
	./run.sh tbm 1 2 3 4 5 6 7 8 || \
	echo -e "Tests failed"

cmd: cmd.c
	make -C $(LIB) $(ALG) STRIDE=$(STRIDE) > /dev/null 2>&1
	$(CC) $(FLAGS) cmd.c $(LIB)/lpm.a -o $(ALG) -pthread > /dev/null 2>&1

clean:
	rm bspl tbm
