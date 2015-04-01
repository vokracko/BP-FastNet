OUTPUT=""
LIB=../../lib/pm
ALG="ac"

.PHONY: .FORCE

all:
	# ./plot.sh "4" "bspl tbm" "1k 10k 100k" "1 2 3 4 5 6 7 8 9 10 11 12 13 14" #TODO dořešit co se bude porovnavat

bin: .FORCE bin.c
	make -C $(LIB) ALG=$(ALG)
	gcc -std=c99 bin.c $(LIB)/pm.a -o $(ALG)

.FORCE:

clean:
	rm -rf aho-corasic *.png *.pdf output/*/* 2>/dev/null
