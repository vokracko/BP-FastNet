CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3 -O0
LIB=../../lib/pm

all:
	@make ac

ac: .FORCE
	@../run.sh pm ac

bin: bin.c
	make -C $(LIB) lib ALG=ac
	$(CC) $(FLAGS) bin.c $(LIB)/pm.a -o ac -pthread

clean:
	rm -rf ac 2>/dev/null

.FORCE:
