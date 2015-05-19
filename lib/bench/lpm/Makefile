CC=gcc
STRIDE=2
ALG=tbm
IP=4
LIB=../../src/lpm

.PHONY: .FORCE

all:
	@make ipv4
	@make ipv6

ipv4: .FORCE
	./plot.sh "4" "bspl tbm" "1k 10k 100k" "1 2 3 4 5 6 7 8"

ipv6: .FORCE
	./plot.sh "6" "bspl tbm" "1k 10k" "1 2 3 4 5 6 7 8"

bin: .FORCE bin.c
	make -C $(LIB) lib ALG=$(ALG) STRIDE=$(STRIDE)
	$(CC) -std=c99 bin.c $(LIB)/lpm.a -o $(ALG) -DIPv$(IP)

.FORCE:

clean:
	rm -rf bspl tbm *.dat *.png *.pdf output/*/* 2>/dev/null
