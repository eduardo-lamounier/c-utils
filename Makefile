CC = gcc
FLAGS = -Wall -Wextra -g

string-view:
	mkdir -p ./bin
	$(CC) $(FLAGS) ./tests/string-view.c -o ./bin/string-view
static-arena:
	mkdir -p ./bin
	$(CC) $(FLAGS) ./tests/static-arena.c -o ./bin/static-arena
dynamic-arena:
	mkdir -p ./bin
	$(CC) $(FLAGS) ./tests/dynamic-arena.c -o ./bin/dynamic-arena

clean:
	rm -rf bin/*

all: string-view static-arena dynamic-arena
