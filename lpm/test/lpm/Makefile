CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3 -O0
LIB=../../lib/

all:
	@make bspl
	@make tbm

bspl: .FORCE
	@./run.sh bspl || echo -e "Tests failed"

tbm: .FORCE
	@./run.sh tbm 1 2 3 4 || echo -e "Tests failed"
	# TODO FIXME doplnit 5 6 7 8 až se rozšíří velikost typů TBM bitmap

cmd: cmd.c
	make -C $(LIB) $(ALG) STRIDE=$(STRIDE)
	$(CC) $(FLAGS) cmd.c $(LIB)/lpm.a -o $(ALG) -pthread

clean:
	rm -rf bspl tbm 2>/dev/null

.FORCE:
