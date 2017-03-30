editor: main.o editor_utils.o editor_line.o editor_routines.o editor_getters.o editor_invokers.o editor_err.o
	gcc -o editor $^

%.o: %.c
	gcc -std=c89 -Wall -Wextra -ansi -pedantic -c $<
	astyle --style=ansi $(CUR_DIR)/$<

CUR_DIR = $(shell pwd)

%.form: %.c
	astyle --style=ansi $(CUR_DIR)/$<

format: main.form editor_getters.form editor_invokers.form editor_line.form editor_routines.form editor_utils.form editor_err.c

tests:
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./editor < test1

clean:
	rm -f *.c.orig *.o editor