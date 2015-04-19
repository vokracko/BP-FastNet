CC=gcc
STRIDE=2
FLAGS=-std=c99 -Wall -Wextra -g3

all: FLAGS=-std=c99 -Wall -Wextra -g3 -DNDEBUG
all: list

list:
	$(CC) $(FLAGS) -c list.c -o list.o

stack:
	$(CC) $(FLAGS) -c stack.c -o stack.o

clean:
	rm -rf *.o 2>/dev/null
