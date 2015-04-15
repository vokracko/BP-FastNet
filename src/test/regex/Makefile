CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3 -O0
LIB=../../lib/regex

all:
	@make dfa

dfa: .FORCE
	@../run.sh regex dfa

bin: bin.c
	make -C $(LIB) lib ALG=dfa
	$(CC) $(FLAGS) bin.c $(LIB)/regex.a -o dfa -pthread

clean:
	rm -rf dfa 2>/dev/null

.FORCE:
