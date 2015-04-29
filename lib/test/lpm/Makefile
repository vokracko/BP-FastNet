CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3 -O0
LIB=../../src/lpm/

all:
	@make bspl
	@make tbm

bspl: .FORCE
	@../run.sh lpm bspl || echo -e "Tests failed"

tbm: .FORCE
	@../run.sh lpm tbm 1 2 3 4 5 6 7 8 || echo -e "Tests failed"

bin: bin.c
	make -C $(LIB) lib ALG=$(ALG) STRIDE=$(STRIDE)
	$(CC) $(FLAGS) bin.c $(LIB)/lpm.a -o $(ALG) -pthread

clean:
	rm -rf bspl tbm 2>/dev/null

.FORCE:
