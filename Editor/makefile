all: format prog

%.o: %.c
	gcc -g -std=c89 -Wall -Wextra -ansi -pedantic -c $<
	astyle --style=ansi $(CUR_DIR)/$<

prog: main.o
	gcc -g -o editor $^

CUR_DIR = $(shell pwd)

%.form: %.c
	astyle --style=ansi $(CUR_DIR)/$<

format: main.form

tests:
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./editor < test1

clean:
	rm -f *.c.orig *.o editor