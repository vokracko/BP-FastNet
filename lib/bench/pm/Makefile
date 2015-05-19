CC=gcc
LIB=../../src/pm
ALG="ac"

.PHONY: .FORCE

all:
	./plot.sh

bin: .FORCE bin.c
	make -C $(LIB) lib ALG=$(ALG)
	$(CC) -std=c99 data.c bin.c -g3  $(LIB)/pm.a -o $(ALG)

.FORCE:

clean:
	rm -rf ac *.png *.pdf output/* 2>/dev/null
