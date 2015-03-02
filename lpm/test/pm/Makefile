CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3 -O0
LIB=../../lib/

all:
	@make aho-corasic
	
aho-corasic: .FORCE bin

bin: bin.c
	make -C $(LIB) aho-corasic
	$(CC) $(FLAGS) bin.c $(LIB)/aho-corasic.a -o aho-corasic -pthread

clean:
	rm -rf aho-corasic 2>/dev/null

.FORCE:
