OUTPUT=""
LIB=../../src/pm
ALG="ac"

.PHONY: .FORCE

all:
	./plot.sh "ac"

bin: .FORCE bin.c
	make -C $(LIB) ALG=$(ALG)
	gcc -std=c99 bin.c $(LIB)/pm.a -o $(ALG)

.FORCE:

clean:
	rm -rf aho-corasic *.png *.pdf output/*/* 2>/dev/null
