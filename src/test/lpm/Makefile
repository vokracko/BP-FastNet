CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3 -O0
LIB=../../lib/lpm/

all:
	@make bspl
	@make tbm

bspl: .FORCE
	@./run.sh bspl || echo -e "Tests failed"

tbm: .FORCE
	@./run.sh tbm 1 2 3 4 5 6 7 8 || echo -e "Tests failed"

cmd: cmd.c
	make -C $(LIB) lib ALG=$(ALG) STRIDE=$(STRIDE)
	$(CC) $(FLAGS) cmd.c $(LIB)/lpm.a -o $(ALG) -pthread

clean:
	rm -rf bspl tbm 2>/dev/null

.FORCE:
