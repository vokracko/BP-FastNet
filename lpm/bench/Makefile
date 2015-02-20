STRIDE=2
OUTPUT=""

.PHONY: .FORCE

all:
	./plot.sh 4 > ipv4-plot.data
	gnuplot plot.plt < ipv4-plot.data

bin: .FORCE bin.c
	make -C ../lib $(ALG) STRIDE=$(STRIDE)
	gcc -std=c99 bin.c ../lib/lpm.a -o $(ALG)

.FORCE:

clean:
	rm -rf bspl tbm *.dat *.png *.pdf 2>/dev/null
