STRIDE=2
OUTPUT=""
LIB=../../lib/lpm

.PHONY: .FORCE

all:
	./plot.sh "4" "bspl tbm" "1k 10k 100k" "1 2 3 4 5 6 7 8"

bin: .FORCE bin.c
	make -C $(LIB) ALG=$(ALG) STRIDE=$(STRIDE)
	gcc -std=c99 bin.c $(LIB)/lpm.a -o $(ALG)

.FORCE:

clean:
	rm -rf bspl tbm *.dat *.png *.pdf output/*/* 2>/dev/null
