setup:
	gcc -std=gnu99 -g -Wall -o smallsh main.c

clean:
	rm -f smallsh

debug:
	valgrind -s --leak-check=yes --track-origins=yes --show-reachable=yes ./smallsh
