CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3 -O0

.PHONY: lpm

all: lpm pm

lpm:
	make -C lpm/

pm:
	make -C pm/

clean:
	make -C lpm/ clean
	make -C pm/ clean
