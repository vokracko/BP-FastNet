STRIDE=2
OUTPUT=""

.PHONY: .FORCE

all:
	./plot.sh "4" "bspl tbm" "1k 10k 100k" "1 2 3 4 5 6 7 8 9 10 11 12 13 14"

bin: .FORCE bin.c
	make -C ../lib/lpm/ ALG=$(ALG) STRIDE=$(STRIDE)
	gcc -std=c99 bin.c ../lib/lpm/lpm.a -o $(ALG)

.FORCE:

clean:
	rm -rf bspl tbm *.dat *.png *.pdf 2>/dev/null
