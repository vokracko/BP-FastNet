CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3

all: bspl

bspl: bspl.o
	ar rcs lpm.a $<

clean:
	rm -rf *.o *.a *.so 2>/dev/null

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@
