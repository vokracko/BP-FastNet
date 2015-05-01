OUTPUT=""
LIB=../../src/regex
ALG="nfa"

.PHONY: .FORCE

all:
	./plot.sh "nfa dfa"

bin: .FORCE bin.c
	make -C $(LIB) lib
	gcc -std=c99 -g3 data.c bin.c -D$(ALG) $(LIB)/regex.a -o $(ALG)

.FORCE:

clean:
	rm -rf nfa dfa *.png *.pdf output/*/* 2>/dev/null
