OUTPUT=""
LIB=../../src/pm
ALG="ac"

.PHONY: .FORCE

all:
	./plot.sh

bin: .FORCE bin.c
	make -C $(LIB) ALG=$(ALG)
	gcc -std=c99 bin.c  data.c $(LIB)/pm.a -o $(ALG)

.FORCE:

clean:
	rm -rf ac *.png *.pdf output/* 2>/dev/null
