CC=gcc
FLAGS=-std=c99 -Wall -Wextra -g3

all: FLAGS=-std=c99 -Wall -Wextra -g3 -DNDEBUG
all: list stack queue

queue:
	$(CC) $(FLAGS) -c queue.c -o queue.o

clean:
	rm -rf *.o 2>/dev/null
