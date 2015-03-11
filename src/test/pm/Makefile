CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3 -O0
LIB=../../lib/pm

all:
	@make aho-corasic

aho-corasic: .FORCE bin
	@../run.sh pm aho-corasic

bin: bin.c
	make -C $(LIB) ALG=aho-corasic
	$(CC) $(FLAGS) bin.c $(LIB)/pm.a -o aho-corasic -pthread

clean:
	rm -rf aho-corasic 2>/dev/null

.FORCE:
