CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3 -O0
LIB=../../src/regex

all:
	@make nfa
	@make dfa

dfa: .FORCE
	@../run.sh regex dfa || echo -e "Tests failed"

nfa: .FORCE
	@../run.sh regex nfa || echo -e "Tests failed"

bin: bin.c
	make -C $(LIB) lib ALG=dfa
	$(CC) $(FLAGS) bin.c -D$(ALG) $(LIB)/regex.a -o $(ALG) -pthread

clean:
	rm -rf dfa 2>/dev/null

.FORCE:
