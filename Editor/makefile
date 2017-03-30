CUR_DIR = $(shell pwd)
OBJS = $(patsubst %.c, %.o, $(wildcard *.c))
FROMS = $(patsubst %.c, %.form, $(wildcard *.c))

editor: $(OBJS)
	gcc -o editor $^

%.o: %.c
	gcc -std=c89 -Wall -Wextra -ansi -pedantic -c $<
	astyle --style=ansi $(CUR_DIR)/$<


%.form: %.c
	astyle --style=ansi $(CUR_DIR)/$<

format: $(FROMS)

tests:
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./editor < test1

clean:
	rm -f *.c.orig *.o editor